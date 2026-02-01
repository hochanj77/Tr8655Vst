import React, { useState, useEffect, useRef, useMemo } from 'react';
import { motion, AnimatePresence } from 'motion/react';
import { Visualizer } from './Visualizer';
import { Knob } from './Knob';
import { Play, Pause, Activity, Zap, Waves, Volume2, Settings2, ChevronLeft, ChevronRight } from 'lucide-react';

const PRESET_CATEGORIES = {
  "808 LIST": [
    { name: "DEEP SUB 01", gain: 45, drive: 12, punch: 82, thresh: 18 },
    { name: "PUNCHY 808", gain: 38, drive: 45, punch: 94, thresh: 24 },
    { name: "DIRTY HARMONICS", gain: 52, drive: 68, punch: 55, thresh: 32 },
    { name: "SMOOTH SINE", gain: 28, drive: 4, punch: 35, thresh: 12 },
    { name: "CRUNCH BASS", gain: 40, drive: 55, punch: 70, thresh: 20 },
  ],
  "X2 PRESETS": [
    { name: "X2 ULTRA LOW", gain: 65, drive: 15, punch: 88, thresh: 10 },
    { name: "X2 AGGRESSIVE", gain: 55, drive: 85, punch: 72, thresh: 25 },
    { name: "X2 CLEAN SUB", gain: 35, drive: 5, punch: 45, thresh: 12 },
    { name: "X2 METALLIC", gain: 48, drive: 65, punch: 62, thresh: 28 },
    { name: "X2 PURE DRIVE", gain: 42, drive: 95, punch: 50, thresh: 15 },
  ]
};

const FLAT_PRESETS = [...PRESET_CATEGORIES["808 LIST"], ...PRESET_CATEGORIES["X2 PRESETS"]];

const Screw = ({ className }: { className?: string }) => (
  <div className={`size-3 rounded-full bg-neutral-800 border-b border-white/5 shadow-[inset_0_1px_2px_rgba(0,0,0,0.8)] flex items-center justify-center ${className}`}>
    <div className="w-[1px] h-2 bg-neutral-900 rotate-45" />
  </div>
);

const VerticalMeter = ({ value, label }: { value: number; label: string }) => {
  const segments = 32;
  const [peak, setPeak] = useState(0);
  const peakTimeout = useRef<NodeJS.Timeout | null>(null);

  useEffect(() => {
    if (value > peak) {
      setPeak(value);
      if (peakTimeout.current) clearTimeout(peakTimeout.current);
      peakTimeout.current = setTimeout(() => {
        setPeak(0);
      }, 1000);
    }
  }, [value, peak]);

  return (
    <div className="flex flex-col items-center gap-2">
      <div className="relative p-[3px] bg-black/90 rounded-[3px] shadow-[inset_0_1px_6px_rgba(0,0,0,1)] border border-white/5">
        <div className="flex flex-col-reverse gap-[1.5px]">
          {Array.from({ length: segments }).map((_, i) => {
            const threshold = (i + 1) / segments;
            const isActive = value >= threshold;
            const isPeak = Math.floor(peak * segments) === i + 1;
            
            let color = "#064e3b"; // Very dark Green (Off)
            let activeColor = "#10b981"; // Vibrant Emerald (On)
            let shadowColor = "rgba(16, 185, 129, 0.4)";

            if (i >= 26) {
              color = "#7f1d1d"; // Dark Red (Off)
              activeColor = "#ef4444"; // Bright Red (On)
              shadowColor = "rgba(239, 68, 68, 0.5)";
            } else if (i >= 20) {
              color = "#713f12"; // Dark Amber (Off)
              activeColor = "#f59e0b"; // Vibrant Amber (On)
              shadowColor = "rgba(245, 158, 11, 0.4)";
            }

            return (
              <motion.div
                key={i}
                className="w-4 h-[3px] rounded-[0.5px] transition-colors duration-75"
                animate={{
                  backgroundColor: (isActive || isPeak) ? activeColor : color,
                  boxShadow: (isActive || isPeak) ? `0 0 6px ${shadowColor}` : "none",
                  opacity: isActive ? 1 : (isPeak ? 0.8 : 0.15)
                }}
              />
            );
          })}
        </div>
      </div>
      <span className="text-[7px] font-mono font-bold text-neutral-500 uppercase tracking-[0.25em]">{label}</span>
    </div>
  );
};

export const BassPlugin: React.FC = () => {
  const [isPlaying, setIsPlaying] = useState(false);
  const [intensity, setIntensity] = useState(0);
  const [bassIntensity, setBassIntensity] = useState(0);
  
  const audioContext = useRef<AudioContext | null>(null);
  const analyser = useRef<AnalyserNode | null>(null);
  const animationFrame = useRef<number>(0);

  const [drive, setDrive] = useState(25);
  const [gain, setGain] = useState(40);
  const [punch, setPunch] = useState(65);
  const [thresh, setThresh] = useState(20);
  const [presetIndex, setPresetIndex] = useState(0);
  const [isGlitching, setIsGlitching] = useState(false);
  const [isMenuOpen, setIsMenuOpen] = useState(false);

  useEffect(() => {
    const glitchInterval = setInterval(() => {
      setIsGlitching(true);
      setTimeout(() => setIsGlitching(false), 800);
    }, 10000);
    return () => clearInterval(glitchInterval);
  }, []);

  const selectPreset = (index: number) => {
    setPresetIndex(index);
    const p = FLAT_PRESETS[index];
    setGain(p.gain);
    setDrive(p.drive);
    setPunch(p.punch);
    setThresh(p.thresh);
    setIsMenuOpen(false);
  };

  const changePreset = (dir: number) => {
    const next = (presetIndex + dir + FLAT_PRESETS.length) % FLAT_PRESETS.length;
    selectPreset(next);
  };

  const toggleAudio = async () => {
    if (!audioContext.current || audioContext.current.state === 'closed') {
      audioContext.current = new (window.AudioContext || (window as any).webkitAudioContext)();
      analyser.current = audioContext.current.createAnalyser();
      analyser.current.fftSize = 256;
      
      const startOscillator = () => {
        if (!audioContext.current || !analyser.current) return;
        
        const freqData = new Uint8Array(analyser.current.frequencyBinCount);
        let smoothedIntensity = 0;
        let smoothedBass = 0;
        const decayRate = 0.12; // Professional decay speed
        
        const update = () => {
          if (!analyser.current) return;
          analyser.current.getByteFrequencyData(freqData);
          
          // Better peak-focused sensitivity
          let sum = 0;
          for(let i = 0; i < freqData.length; i++) sum += freqData[i];
          const targetIntensity = (sum / freqData.length / 255) * 1.8;
          
          let bassSum = 0;
          for(let i = 0; i < 20; i++) bassSum += freqData[i];
          const targetBass = (bassSum / 20 / 255) * 2.2;
          
          // Ballistics: Instant Attack, Logarithmic Decay
          if (targetIntensity > smoothedIntensity) {
            smoothedIntensity = targetIntensity;
          } else {
            smoothedIntensity -= (smoothedIntensity - targetIntensity) * decayRate;
          }

          if (targetBass > smoothedBass) {
            smoothedBass = targetBass;
          } else {
            smoothedBass -= (smoothedBass - targetBass) * decayRate;
          }
          
          setIntensity(Math.min(1, smoothedIntensity));
          setBassIntensity(Math.min(1, smoothedBass));
          
          animationFrame.current = requestAnimationFrame(update);
        };
        
        const osc = audioContext.current.createOscillator();
        const gainNode = audioContext.current.createGain();
        osc.type = 'sine';
        osc.frequency.setValueAtTime(45, audioContext.current.currentTime);
        gainNode.gain.setValueAtTime(0.4, audioContext.current.currentTime);
        
        osc.connect(gainNode);
        gainNode.connect(analyser.current);
        analyser.current.connect(audioContext.current.destination);
        
        osc.start();
        update();
      };
      
      startOscillator();
    }
    
    if (audioContext.current.state === 'suspended') {
      await audioContext.current.resume();
    }
    
    setIsPlaying(!isPlaying);
  };

  useEffect(() => {
    return () => {
      if (animationFrame.current) cancelAnimationFrame(animationFrame.current);
      if (audioContext.current) {
        const ctx = audioContext.current;
        audioContext.current = null;
        if (ctx.state !== 'closed') ctx.close().catch(() => {});
      }
    };
  }, []);

  useEffect(() => {
    if (!isPlaying) {
      const interval = setInterval(() => {
        setIntensity(prev => Math.max(0, prev - 0.08));
        setBassIntensity(prev => Math.max(0, prev - 0.08));
      }, 50);
      return () => clearInterval(interval);
    }
  }, [isPlaying]);

  const effectiveIntensity = useMemo(() => {
    const gainBoost = gain / 100;
    const driveCrunch = drive / 100;
    return Math.min(1, intensity * (1 + gainBoost + driveCrunch * 0.8));
  }, [intensity, gain, drive]);

  const effectiveBassIntensity = useMemo(() => {
    const punchBoost = punch / 100;
    return Math.min(1, bassIntensity * (1 + punchBoost));
  }, [bassIntensity, punch]);

  return (
    <div className="min-h-screen bg-[#020205] flex items-center justify-center p-4 font-sans selection:bg-blue-500/30">
      <motion.div 
        className="relative w-full max-w-5xl rounded-xl border border-white/5 shadow-[0_50px_100px_rgba(0,0,0,0.9)] overflow-hidden"
        style={{
          background: 'linear-gradient(135deg, #2a2a2e 0%, #1a1a1e 35%, #121215 100%)',
        }}
      >
        {/* Anodic Shine / Top Highlight */}
        <div className="absolute inset-x-0 top-0 h-[1px] bg-gradient-to-r from-transparent via-white/10 to-transparent" />
        
        {/* Smooth Surface Sheen */}
        <div className="absolute inset-0 bg-gradient-to-b from-white/[0.03] to-transparent pointer-events-none" />
        
        {/* Subtle Grain (Keep but very faint for that smooth satin metal feel) */}
        <div className="absolute inset-0 opacity-[0.02] pointer-events-none mix-blend-overlay bg-[url('https://grainy-gradients.vercel.app/noise.svg')]" />
        
        {/* Screws */}
        <Screw className="absolute top-4 left-4" />
        <Screw className="absolute top-4 right-4" />
        <Screw className="absolute bottom-4 left-4" />
        <Screw className="absolute bottom-4 right-4" />

        {/* Header Section */}
        <div className="relative px-10 pt-10 pb-6 grid grid-cols-3 items-center border-b border-black/40">
          <div className="flex items-center gap-6">
            <div className="relative p-[1px] rounded-lg bg-gradient-to-b from-white/10 to-transparent shadow-xl">
               <div className="bg-[#08080a] px-8 py-3 rounded-[7px] border border-black shadow-inner flex flex-col items-center">
                  <div className="relative">
                    <motion.h1 
                      className="text-3xl font-mono font-black tracking-tighter italic text-blue-500 relative z-10"
                      animate={{ 
                        textShadow: isPlaying ? '0 0 15px rgba(59,130,246,0.8)' : '0 0 5px rgba(59,130,246,0.2)',
                        x: isGlitching ? [-2, 2, -5, 8, -2, 0] : 0,
                        y: isGlitching ? [0, -1, 1, 0] : 0,
                        opacity: isGlitching ? [1, 0.4, 0.8, 0.2, 1, 0.5, 1] : 1,
                        skewX: isGlitching ? [0, -10, 20, -15, 0] : 0,
                        scaleY: isGlitching ? [1, 1.2, 0.8, 1.1, 1] : 1,
                        filter: isGlitching 
                          ? ['contrast(200%) brightness(150%)', 'contrast(50%) brightness(50%)', 'contrast(100%) brightness(100%)'] 
                          : 'none'
                      }}
                      transition={isGlitching ? { duration: 0.4, repeat: 1 } : { duration: 0.3 }}
                    >
                      TR-8655
                    </motion.h1>
                    
                    {/* Signal Loss "Scanlines" Overlay */}
                    <AnimatePresence>
                      {isGlitching && (
                        <motion.div 
                          initial={{ opacity: 0 }}
                          animate={{ opacity: [0, 0.4, 0.1, 0.5, 0] }}
                          exit={{ opacity: 0 }}
                          className="absolute inset-0 z-20 pointer-events-none bg-white/10 mix-blend-overlay overflow-hidden"
                        >
                          <motion.div 
                            animate={{ y: [-40, 40] }}
                            transition={{ repeat: Infinity, duration: 0.1 }}
                            className="h-[2px] w-full bg-white/40 shadow-[0_0_10px_white]"
                          />
                        </motion.div>
                      )}
                    </AnimatePresence>

                    {/* Chromatic Aberration Layers during glitch */}
                    <AnimatePresence>
                      {isGlitching && (
                        <>
                          <motion.h1 
                            initial={{ opacity: 0 }}
                            animate={{ 
                              opacity: [0, 1, 0.3, 0.8, 0], 
                              x: [-15, 10, -5, 20, -10],
                              skewX: [0, 20, -20, 0]
                            }}
                            transition={{ duration: 0.4 }}
                            exit={{ opacity: 0 }}
                            className="absolute inset-0 text-3xl font-mono font-black tracking-tighter italic text-red-500 mix-blend-screen pointer-events-none"
                          >
                            TR-8655
                          </motion.h1>
                          <motion.h1 
                            initial={{ opacity: 0 }}
                            animate={{ 
                              opacity: [0, 1, 0.5, 0.9, 0], 
                              x: [15, -12, 8, -18, 12],
                              skewX: [0, -25, 15, 0]
                            }}
                            transition={{ duration: 0.4 }}
                            exit={{ opacity: 0 }}
                            className="absolute inset-0 text-3xl font-mono font-black tracking-tighter italic text-cyan-500 mix-blend-screen pointer-events-none"
                          >
                            TR-8655
                          </motion.h1>
                        </>
                      )}
                    </AnimatePresence>
                  </div>
                  <p className="text-[7px] text-neutral-600 font-mono uppercase tracking-[0.6em] mt-1">Oasis Creative Labs</p>
               </div>
            </div>
          </div>

          {/* Center Preset Selector */}
          <div className="flex flex-col items-center gap-1 relative">
            <span className="text-[8px] text-neutral-500 font-bold uppercase tracking-[0.3em]">Patch Memory</span>
            <div className="flex items-center bg-black/60 rounded-md border border-white/5 p-1 gap-1 shadow-inner relative z-40">
              <button 
                onClick={(e) => { e.stopPropagation(); changePreset(-1); }}
                className="p-1.5 hover:bg-white/5 rounded transition-colors text-neutral-400 hover:text-blue-400"
              >
                <ChevronLeft size={16} />
              </button>
              
              <button 
                onClick={() => setIsMenuOpen(!isMenuOpen)}
                className="w-44 h-10 bg-[#050505] rounded flex items-center justify-center border border-white/5 relative overflow-hidden group cursor-pointer"
              >
                <AnimatePresence mode="wait">
                  <motion.span 
                    key={presetIndex}
                    initial={{ opacity: 0, y: 10 }}
                    animate={{ opacity: 1, y: 0 }}
                    exit={{ opacity: 0, y: -10 }}
                    className="text-[11px] font-mono font-bold text-blue-500 tracking-[0.1em] z-10"
                  >
                    {FLAT_PRESETS[presetIndex].name}
                  </motion.span>
                </AnimatePresence>
                <div className="absolute inset-0 bg-gradient-to-b from-white/[0.05] to-transparent pointer-events-none group-hover:from-white/[0.1] transition-all" />
                
                {/* Subtle Glow */}
                <div className="absolute inset-0 bg-blue-500/5 opacity-0 group-hover:opacity-100 transition-opacity" />
              </button>

              <button 
                onClick={(e) => { e.stopPropagation(); changePreset(1); }}
                className="p-1.5 hover:bg-white/5 rounded transition-colors text-neutral-400 hover:text-blue-400"
              >
                <ChevronRight size={16} />
              </button>
            </div>

            {/* Dropdown Menu */}
            <AnimatePresence>
              {isMenuOpen && (
                <>
                  <div className="fixed inset-0 z-40" onClick={() => setIsMenuOpen(false)} />
                  <motion.div 
                    initial={{ opacity: 0, y: -10, scale: 0.95 }}
                    animate={{ opacity: 1, y: 0, scale: 1 }}
                    exit={{ opacity: 0, y: -10, scale: 0.95 }}
                    className="absolute top-full mt-2 w-72 bg-[#0a0a0c] border border-white/10 rounded-lg shadow-[0_20px_50px_rgba(0,0,0,0.8)] z-50 overflow-hidden"
                  >
                    <div className="absolute inset-0 bg-gradient-to-b from-white/[0.02] to-transparent pointer-events-none" />
                    
                    <div className="max-h-96 overflow-y-auto p-2 scrollbar-thin scrollbar-thumb-white/10">
                      {Object.entries(PRESET_CATEGORIES).map(([category, items]) => (
                        <div key={category} className="mb-4 last:mb-0">
                          <div className="px-3 py-1 mb-1 border-b border-white/5">
                            <span className="text-[9px] font-bold text-neutral-500 uppercase tracking-[0.3em]">{category}</span>
                          </div>
                          <div className="space-y-0.5">
                            {items.map((item) => {
                              const globalIndex = FLAT_PRESETS.findIndex(p => p.name === item.name);
                              const isActive = presetIndex === globalIndex;
                              return (
                                <button
                                  key={item.name}
                                  onClick={() => selectPreset(globalIndex)}
                                  className={`w-full flex items-center justify-between px-3 py-2 rounded transition-all text-left group ${
                                    isActive 
                                    ? 'bg-blue-500/20 text-blue-400' 
                                    : 'text-neutral-400 hover:bg-white/5 hover:text-neutral-200'
                                  }`}
                                >
                                  <span className="text-[10px] font-mono font-bold tracking-wider">{item.name}</span>
                                  {isActive && <div className="size-1.5 rounded-full bg-blue-500 shadow-[0_0_8px_rgba(59,130,246,0.8)]" />}
                                </button>
                              );
                            })}
                          </div>
                        </div>
                      ))}
                    </div>
                  </motion.div>
                </>
              )}
            </AnimatePresence>
          </div>
          
          <div className="flex justify-end items-center gap-8">
            <div className="flex flex-col items-end gap-1">
              <span className="text-[9px] text-neutral-500 font-bold uppercase tracking-widest">Power Control</span>
              <button 
                onClick={toggleAudio}
                className={`relative px-10 py-3 rounded-md border border-white/5 transition-all duration-300 overflow-hidden shadow-lg ${
                  isPlaying 
                  ? 'bg-red-500/10 text-red-500 border-red-500/20' 
                  : 'bg-white/5 text-blue-400 border-white/5'
                }`}
              >
                <div className="relative z-10 flex items-center gap-2 font-mono text-xs font-bold uppercase tracking-widest">
                  {isPlaying ? <Pause size={14} /> : <Play size={14} fill="currentColor" />}
                  {isPlaying ? 'Active' : 'Engage'}
                </div>
                {/* Active Light */}
                <motion.div 
                  className={`absolute top-1 right-2 size-1.5 rounded-full ${isPlaying ? 'bg-red-500 shadow-[0_0_8px_rgba(239,68,68,1)]' : 'bg-neutral-800'}`}
                  animate={isPlaying ? { opacity: [1, 0.4, 1] } : {}}
                  transition={{ duration: 1, repeat: Infinity }}
                />
              </button>
            </div>
          </div>
        </div>

        {/* Main Interface Grid */}
        <div className="relative grid grid-cols-1 lg:grid-cols-12 gap-0">
          
          {/* Left Panel */}
          <div className="lg:col-span-3 p-8 border-r border-black/40 bg-black/10">
            <div className="space-y-12">
              <div className="space-y-6">
                <div className="flex items-center gap-2 text-neutral-400 border-b border-white/5 pb-2">
                  <Zap size={12} className="text-blue-500" />
                  <span className="text-[10px] font-bold uppercase tracking-[0.3em]">Signal</span>
                </div>
                <div className="grid grid-cols-2 gap-y-10">
                  <Knob label="Gain" value={gain} onChange={setGain} min={0} max={100} unit="dB" />
                  <Knob label="Drive" value={drive} onChange={setDrive} min={0} max={100} unit="%" />
                </div>
              </div>

              <div className="space-y-6">
                <div className="flex items-center gap-2 text-neutral-400 border-b border-white/5 pb-2">
                  <Waves size={12} className="text-blue-500" />
                  <span className="text-[10px] font-bold uppercase tracking-[0.3em]">Dynamics</span>
                </div>
                <div className="grid grid-cols-2 gap-y-10">
                  <Knob label="Thresh" value={thresh} onChange={setThresh} min={0} max={100} unit="dB" />
                  <Knob label="Ratio" defaultValue={40} unit=":1" />
                </div>
              </div>
            </div>
          </div>

          <div className="lg:col-span-6 flex flex-col items-center justify-center py-4 px-4 relative min-h-[400px]">
            {/* Meters adjusted for the larger visualizer */}
            <div className="absolute left-8 top-1/2 -translate-y-1/2">
              <VerticalMeter value={effectiveIntensity} label="IN" />
            </div>
            <div className="absolute right-8 top-1/2 -translate-y-1/2">
              <VerticalMeter value={effectiveBassIntensity} label="SUB" />
            </div>

            <div className="relative">
               {/* Tight Recessed Panel - Adjusted for larger scale */}
               <div className="absolute -inset-6 bg-gradient-to-br from-black/90 to-black/50 rounded-full shadow-[inset_0_4px_60px_rgba(0,0,0,1)] border border-white/5 pointer-events-none" />
               <Visualizer intensity={effectiveIntensity} bassIntensity={effectiveBassIntensity} />
            </div>
          </div>

          {/* Right Panel */}
          <div className="lg:col-span-3 p-8 border-l border-black/40 bg-black/10">
            <div className="space-y-12">
              <div className="space-y-6">
                <div className="flex items-center gap-2 text-neutral-400 border-b border-white/5 pb-2">
                  <Activity size={12} className="text-blue-500" />
                  <span className="text-[10px] font-bold uppercase tracking-[0.3em]">Equalizer</span>
                </div>
                <div className="grid grid-cols-2 gap-y-10">
                  <Knob label="Sub" defaultValue={75} unit="Hz" />
                  <Knob label="Punch" value={punch} onChange={setPunch} min={0} max={100} unit="%" />
                </div>
              </div>

              <div className="space-y-6">
                <div className="flex items-center gap-2 text-neutral-400 border-b border-white/5 pb-2">
                  <Volume2 size={12} className="text-blue-500" />
                  <span className="text-[10px] font-bold uppercase tracking-[0.3em]">Master</span>
                </div>
                <div className="grid grid-cols-2 gap-y-10">
                  <Knob label="Mix" defaultValue={100} unit="%" />
                  <Knob label="Output" defaultValue={50} unit="dB" />
                </div>
              </div>
            </div>
          </div>
        </div>

        {/* Status Bar */}
        <div className="bg-black/60 px-10 py-4 flex justify-between items-center text-[9px] font-mono text-neutral-500 border-t border-black/40 relative z-20">
          <div className="flex gap-8 uppercase tracking-[0.2em]">
            <div className="flex items-center gap-2">
              <div className="size-1.5 rounded-full bg-blue-500/50" />
              <span>48 kHz</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="size-1.5 rounded-full bg-blue-500/50" />
              <span>32-bit</span>
            </div>
          </div>
          
          <div className="flex items-center gap-3">
            <span className="uppercase tracking-widest text-neutral-600">CPU</span>
            <div className="w-24 h-1.5 bg-neutral-950 rounded-full border border-white/5 overflow-hidden">
              <motion.div 
                className="h-full bg-gradient-to-r from-blue-600 to-blue-400"
                animate={{ width: isPlaying ? '18%' : '2%' }}
                transition={{ duration: 1 }}
              />
            </div>
            <Settings2 size={12} className="text-neutral-700 ml-2" />
          </div>
        </div>
      </motion.div>
    </div>
  );
};
