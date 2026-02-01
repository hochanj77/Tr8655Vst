import React, { useRef, useMemo } from 'react';
import { motion } from 'motion/react';
import imgAnimationThird from "figma:asset/4afd3b3452e1b94a144686d0fb0bac3550677f44.png";
import imgAnimationThird1 from "figma:asset/9616b57a0a1bff7cab9564ffce393096c5c4dfa2.png";

interface VisualizerProps {
  intensity: number; // 0 to 1
  bassIntensity: number; // 0 to 1
  className?: string;
}

const WaveLayer = ({ 
  index, 
  intensity, 
  bassIntensity, 
  color, 
  opacity, 
  speed, 
  freq, 
  amplitudeMultiplier,
  isFilled = false,
  warpIntensity = 0
}: { 
  index: number; 
  intensity: number; 
  bassIntensity: number;
  color: string;
  opacity: number;
  speed: number;
  freq: number;
  amplitudeMultiplier: number;
  isFilled?: boolean;
  warpIntensity?: number;
}) => {
  const points = 70;
  const size = 260; // Larger internal coordinate space
  const centerY = size / 2;
  const step = size / (points - 1);
  const time = useRef(0);
  
  time.current += speed * (0.8 + intensity * 2);

  const d = useMemo(() => {
    const p = [];
    
    for (let i = 0; i < points; i++) {
      const x = i * step;
      const normalizedX = (i / (points - 1) * 2 - 1);
      const edgeFade = Math.pow(1 - Math.pow(normalizedX, 2), 1.1);
      
      const primaryWave = Math.sin(i * freq + time.current) * (40 + intensity * 120 * amplitudeMultiplier);
      const warp = Math.sin(i * 0.08 + time.current * 0.4) * warpIntensity * 30 * bassIntensity;
      const secondaryWave = Math.sin(i * freq * 1.5 - time.current * 0.8) * (25 + bassIntensity * 60);
      
      const y = centerY + (primaryWave + secondaryWave + warp) * edgeFade;
      p.push(`${i === 0 ? 'M' : 'L'} ${x} ${y}`);
    }
    
    if (isFilled) {
      p.push(`L ${size} ${centerY} L 0 ${centerY} Z`);
    }
    
    return p.join(' ');
  }, [intensity, bassIntensity, freq, amplitudeMultiplier, points, step, isFilled, time.current, warpIntensity, size, centerY]);

  return (
    <motion.path
      d={d}
      stroke={isFilled ? "none" : color}
      fill={isFilled ? color : "none"}
      strokeWidth={isFilled ? 0 : 10 + index * 1.5}
      strokeLinecap="round"
      strokeLinejoin="round"
      initial={false}
      animate={{
        opacity: opacity * (0.8 + intensity * 0.2),
        strokeWidth: isFilled ? 0 : (10 + index * 1.5) * (1 + bassIntensity * 0.8)
      }}
      transition={{ duration: 0.1, ease: "linear" }}
      style={{ filter: isFilled ? 'none' : 'blur(1.5px)' }}
    />
  );
};

export const Visualizer: React.FC<VisualizerProps> = ({ intensity, bassIntensity, className }) => {
  const size = 260; // Increased visualizer size
  const center = size / 2;
  
  return (
    <div className={className || "relative size-[300px] flex items-center justify-center"} data-name="Animation/Visualizer">
      
      {/* Dynamic Light Aura - Reactive to Bass */}
      <motion.div 
        className="absolute size-[280px] rounded-full"
        animate={{ 
          boxShadow: [
            `0 0 ${30 + bassIntensity * 60}px rgba(59, 130, 246, ${0.3 + intensity * 0.5})`,
            `0 0 ${60 + bassIntensity * 100}px rgba(147, 51, 234, ${0.2 + intensity * 0.4})`,
            `0 0 ${30 + bassIntensity * 60}px rgba(59, 130, 246, ${0.3 + intensity * 0.5})`
          ],
          scale: 1 + bassIntensity * 0.15
        }}
        transition={{ duration: 2, repeat: Infinity, ease: "easeInOut" }}
      />
      
      {/* Secondary Soft Glow Layer */}
      <motion.div 
        className="absolute size-[320px] rounded-full bg-blue-500/10 blur-[50px]"
        animate={{ 
          opacity: 0.3 + intensity * 0.7,
          scale: 0.9 + bassIntensity * 0.5
        }}
      />

      <motion.div 
        className="relative size-[260px]"
        animate={{ 
          scale: 1 + intensity * 0.05,
          rotate: intensity * 1
        }}
      >
        {/* Main SVG Circle */}
        <svg className="block size-full relative z-10" fill="none" preserveAspectRatio="none" viewBox={`0 0 ${size} ${size}`}>
          <circle cx={center} cy={center} fill="#02020F" r={center} />
          
          {/* Reactive Waves */}
          <g clipPath="url(#vis_v5_clip)" className="mix-blend-screen">
            <WaveLayer index={0} intensity={intensity} bassIntensity={bassIntensity} color="rgba(29, 78, 216, 0.4)" opacity={0.3} speed={0.02} freq={0.04} amplitudeMultiplier={1.5} isFilled={true} warpIntensity={1} />
            <WaveLayer index={1} intensity={intensity} bassIntensity={bassIntensity} color="rgba(124, 58, 237, 0.2)" opacity={0.2} speed={-0.015} freq={0.06} amplitudeMultiplier={1.2} isFilled={true} warpIntensity={1.5} />
            <WaveLayer index={2} intensity={intensity} bassIntensity={bassIntensity} color="#3B82F6" opacity={0.8} speed={0.04} freq={0.05} amplitudeMultiplier={1.2} warpIntensity={1.2} />
            <WaveLayer index={3} intensity={intensity} bassIntensity={bassIntensity} color="#6366F1" opacity={0.7} speed={-0.03} freq={0.08} amplitudeMultiplier={0.9} warpIntensity={1.8} />
            <WaveLayer index={4} intensity={intensity} bassIntensity={bassIntensity} color="#8B5CF6" opacity={0.6} speed={0.05} freq={0.12} amplitudeMultiplier={0.7} warpIntensity={2.5} />
            
            <motion.path
              d={`M 0 ${center} L ${size} ${center}`}
              stroke="#FFF"
              strokeWidth="6"
              animate={{ 
                opacity: 0.8 + intensity * 0.2,
                strokeWidth: 5 + bassIntensity * 10,
              }}
              style={{ filter: 'blur(4px)' }}
            />
          </g>

          <defs>
            <clipPath id="vis_v5_clip">
              <circle cx={center} cy={center} r={center} />
            </clipPath>
          </defs>
        </svg>

        {/* Clean border ring with its own light glow */}
        <motion.div 
          className="absolute inset-0 rounded-full border border-white/20 pointer-events-none z-30" 
          animate={{ 
            borderColor: [`rgba(255,255,255,0.1)`, `rgba(59,130,246,0.4)`, `rgba(255,255,255,0.1)`]
          }}
          transition={{ duration: 3, repeat: Infinity }}
        />
      </motion.div>
    </div>
  );
};
