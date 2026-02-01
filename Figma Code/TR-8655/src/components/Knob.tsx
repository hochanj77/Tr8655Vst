import React, { useState, useEffect, useRef } from 'react';
import { motion, useSpring } from 'motion/react';

interface KnobProps {
  label: string;
  min?: number;
  max?: number;
  defaultValue?: number;
  value?: number;
  onChange?: (value: number) => void;
  unit?: string;
}

export const Knob: React.FC<KnobProps> = ({ 
  label, 
  min = 0, 
  max = 100, 
  defaultValue = 50, 
  value,
  onChange,
  unit = ""
}) => {
  const [internalValue, setInternalValue] = useState(defaultValue);
  const isDragging = useRef(false);
  const startY = useRef(0);
  const startValueRef = useRef(0);

  const currentVal = value !== undefined ? value : internalValue;

  const rotation = useSpring((currentVal - min) / (max - min) * 270 - 135, {
    stiffness: 400,
    damping: 40
  });

  useEffect(() => {
    rotation.set((currentVal - min) / (max - min) * 270 - 135);
  }, [currentVal, min, max, rotation]);

  const onMouseDown = (e: React.MouseEvent) => {
    isDragging.current = true;
    startY.current = e.clientY;
    startValueRef.current = currentVal;
    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup', onMouseUp);
    e.preventDefault();
  };

  const onMouseMove = (e: MouseEvent) => {
    if (!isDragging.current) return;
    const delta = startY.current - e.clientY;
    const range = max - min;
    const step = range / 150; 
    const newValue = Math.min(max, Math.max(min, startValueRef.current + delta * step));
    
    if (onChange) {
      onChange(newValue);
    } else {
      setInternalValue(newValue);
    }
  };

  const onMouseUp = () => {
    isDragging.current = false;
    window.removeEventListener('mousemove', onMouseMove);
    window.removeEventListener('mouseup', onMouseUp);
  };

  return (
    <div className="flex flex-col items-center gap-3 select-none">
      <div 
        className="relative size-14 cursor-ns-resize group"
        onMouseDown={onMouseDown}
      >
        {/* Smooth Recessed Inset */}
        <div className="absolute inset-[-6px] rounded-full bg-gradient-to-br from-black/80 to-black/20 shadow-[inset_0_2px_10px_rgba(0,0,0,1)] border border-white/5" />
        
        {/* Smooth Knob Body - Liquid Metal Feel */}
        <div className="absolute inset-0 rounded-full bg-gradient-to-b from-neutral-600 via-neutral-800 to-neutral-950 shadow-[0_8px_15px_rgba(0,0,0,0.6),inset_0_1px_1px_rgba(255,255,255,0.2)] border border-black/40" />
        
        {/* Rotating Cap with Shine */}
        <motion.div 
          className="absolute inset-[2px] rounded-full bg-gradient-to-tr from-neutral-800 via-neutral-900 to-neutral-800 shadow-xl flex items-center justify-center overflow-hidden"
          style={{ rotate: rotation }}
        >
          {/* Surface highlight to simulate smooth metal curvature */}
          <div className="absolute inset-0 bg-gradient-to-br from-white/10 to-transparent opacity-50" />
          
          {/* Pointer Line - Neon Blue Glow */}
          <div className="absolute top-[3px] left-1/2 -translate-x-1/2 w-[2px] h-[12px] bg-blue-400 rounded-full shadow-[0_0_10px_rgba(96,165,250,0.8)] z-10" />
        </motion.div>

        {/* Ambient Hover Glow */}
        <div className="absolute inset-[-4px] rounded-full bg-blue-500/0 group-hover:bg-blue-500/[0.03] transition-colors pointer-events-none" />
      </div>
      
      <div className="text-center">
        <p className="text-[9px] font-bold text-neutral-500 uppercase tracking-widest leading-none mb-2">{label}</p>
        <div className="bg-[#08080a] px-2 py-1 rounded border border-white/5 shadow-inner">
          <p className="text-[10px] font-mono text-blue-400/90 tracking-tighter">
            {Math.round(currentVal)}<span className="text-[8px] opacity-40 ml-0.5">{unit}</span>
          </p>
        </div>
      </div>
    </div>
  );
};
