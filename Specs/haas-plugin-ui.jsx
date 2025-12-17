import React, { useState, useEffect, useRef } from 'react';

// Knob Component
const Knob = ({ value, onChange, min = 0, max = 100, size = 80, label, unit = '', color = '#00d4ff', showValue = true }) => {
  const knobRef = useRef(null);
  const [isDragging, setIsDragging] = useState(false);
  const startY = useRef(0);
  const startValue = useRef(0);

  const rotation = ((value - min) / (max - min)) * 270 - 135;

  useEffect(() => {
    const handleMouseMove = (e) => {
      if (!isDragging) return;
      const delta = startY.current - e.clientY;
      const range = max - min;
      const newValue = Math.max(min, Math.min(max, startValue.current + (delta / 200) * range));
      onChange(newValue);
    };

    const handleMouseUp = () => setIsDragging(false);

    if (isDragging) {
      window.addEventListener('mousemove', handleMouseMove);
      window.addEventListener('mouseup', handleMouseUp);
    }
    return () => {
      window.removeEventListener('mousemove', handleMouseMove);
      window.removeEventListener('mouseup', handleMouseUp);
    };
  }, [isDragging, min, max, onChange]);

  const handleMouseDown = (e) => {
    setIsDragging(true);
    startY.current = e.clientY;
    startValue.current = value;
  };

  return (
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '8px' }}>
      <div
        ref={knobRef}
        onMouseDown={handleMouseDown}
        style={{
          width: size,
          height: size,
          borderRadius: '50%',
          background: `
            radial-gradient(circle at 30% 30%, #3a3a3a, #1a1a1a 70%),
            conic-gradient(from -135deg, ${color}22 0deg, ${color}44 ${(rotation + 135) * 0.75}deg, #222 ${(rotation + 135) * 0.75}deg, #222 270deg)
          `,
          boxShadow: `
            inset 0 2px 4px rgba(255,255,255,0.1),
            inset 0 -2px 4px rgba(0,0,0,0.5),
            0 4px 12px rgba(0,0,0,0.5),
            0 0 20px ${color}22
          `,
          cursor: 'grab',
          position: 'relative',
          border: `2px solid #333`,
        }}
      >
        {/* Indicator line */}
        <div
          style={{
            position: 'absolute',
            top: '50%',
            left: '50%',
            width: '2px',
            height: size / 2 - 8,
            background: `linear-gradient(to top, ${color}, ${color}cc)`,
            transformOrigin: 'bottom center',
            transform: `translate(-50%, -100%) rotate(${rotation}deg)`,
            borderRadius: '2px',
            boxShadow: `0 0 8px ${color}`,
          }}
        />
        {/* Center cap */}
        <div
          style={{
            position: 'absolute',
            top: '50%',
            left: '50%',
            transform: 'translate(-50%, -50%)',
            width: size * 0.35,
            height: size * 0.35,
            borderRadius: '50%',
            background: 'radial-gradient(circle at 30% 30%, #444, #222)',
            boxShadow: 'inset 0 1px 2px rgba(255,255,255,0.1)',
          }}
        />
      </div>
      {showValue && (
        <div style={{
          background: '#1a1a1a',
          padding: '4px 12px',
          borderRadius: '4px',
          border: '1px solid #333',
          fontFamily: "'JetBrains Mono', monospace",
          fontSize: '11px',
          color: color,
          minWidth: '60px',
          textAlign: 'center',
        }}>
          {typeof value === 'number' ? value.toFixed(1) : value}{unit}
        </div>
      )}
      {label && (
        <div style={{
          fontFamily: "'Inter', sans-serif",
          fontSize: '11px',
          color: '#888',
          textTransform: 'uppercase',
          letterSpacing: '1px',
        }}>
          {label}
        </div>
      )}
    </div>
  );
};

// Vertical Slider Component
const VerticalSlider = ({ value, onChange, min = 0, max = 100, height = 150, label, color = '#00d4ff' }) => {
  const sliderRef = useRef(null);
  const [isDragging, setIsDragging] = useState(false);

  const percentage = ((value - min) / (max - min)) * 100;

  useEffect(() => {
    const handleMouseMove = (e) => {
      if (!isDragging || !sliderRef.current) return;
      const rect = sliderRef.current.getBoundingClientRect();
      const y = Math.max(0, Math.min(height, rect.bottom - e.clientY));
      const newValue = min + (y / height) * (max - min);
      onChange(newValue);
    };

    const handleMouseUp = () => setIsDragging(false);

    if (isDragging) {
      window.addEventListener('mousemove', handleMouseMove);
      window.addEventListener('mouseup', handleMouseUp);
    }
    return () => {
      window.removeEventListener('mousemove', handleMouseMove);
      window.removeEventListener('mouseup', handleMouseUp);
    };
  }, [isDragging, min, max, height, onChange]);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '8px' }}>
      <div
        ref={sliderRef}
        onMouseDown={() => setIsDragging(true)}
        style={{
          width: '12px',
          height: height,
          background: '#1a1a1a',
          borderRadius: '6px',
          position: 'relative',
          cursor: 'grab',
          border: '1px solid #333',
          boxShadow: 'inset 0 2px 4px rgba(0,0,0,0.5)',
        }}
      >
        {/* Fill */}
        <div
          style={{
            position: 'absolute',
            bottom: 0,
            left: '50%',
            transform: 'translateX(-50%)',
            width: '6px',
            height: `${percentage}%`,
            background: `linear-gradient(to top, ${color}, ${color}88)`,
            borderRadius: '3px',
            boxShadow: `0 0 10px ${color}66`,
          }}
        />
        {/* Handle */}
        <div
          style={{
            position: 'absolute',
            bottom: `calc(${percentage}% - 8px)`,
            left: '50%',
            transform: 'translateX(-50%)',
            width: '20px',
            height: '16px',
            background: 'linear-gradient(to bottom, #555, #333)',
            borderRadius: '3px',
            border: '1px solid #444',
            boxShadow: '0 2px 4px rgba(0,0,0,0.3)',
          }}
        />
      </div>
      {label && (
        <div style={{
          fontFamily: "'Inter', sans-serif",
          fontSize: '10px',
          color: '#666',
          textTransform: 'uppercase',
          letterSpacing: '0.5px',
        }}>
          {label}
        </div>
      )}
    </div>
  );
};

// Phase Correlation Meter
const PhaseCorrelationMeter = ({ correlation, autoPhaseActive, threshold }) => {
  const meterWidth = 200;
  const position = ((correlation + 1) / 2) * meterWidth;
  const thresholdPos = ((threshold + 1) / 2) * meterWidth;

  return (
    <div style={{
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center',
      gap: '8px',
    }}>
      <div style={{
        display: 'flex',
        justifyContent: 'space-between',
        width: meterWidth,
        fontFamily: "'JetBrains Mono', monospace",
        fontSize: '9px',
        color: '#666',
      }}>
        <span>-1</span>
        <span>0</span>
        <span>+1</span>
      </div>
      <div style={{
        width: meterWidth,
        height: '24px',
        background: '#0a0a0a',
        borderRadius: '4px',
        border: '1px solid #333',
        position: 'relative',
        overflow: 'hidden',
      }}>
        {/* Gradient background */}
        <div style={{
          position: 'absolute',
          top: 0,
          left: 0,
          right: 0,
          bottom: 0,
          background: 'linear-gradient(to right, #ff3366 0%, #ff6633 25%, #ffcc00 50%, #66ff66 75%, #00ff88 100%)',
          opacity: 0.3,
        }} />
        {/* Threshold marker */}
        <div style={{
          position: 'absolute',
          top: 0,
          bottom: 0,
          left: thresholdPos,
          width: '2px',
          background: autoPhaseActive ? '#ff6600' : '#444',
          boxShadow: autoPhaseActive ? '0 0 8px #ff6600' : 'none',
        }} />
        {/* Current position indicator */}
        <div style={{
          position: 'absolute',
          top: '50%',
          left: position,
          transform: 'translate(-50%, -50%)',
          width: '4px',
          height: '16px',
          background: correlation < threshold ? '#ff3366' : '#00ff88',
          borderRadius: '2px',
          boxShadow: `0 0 12px ${correlation < threshold ? '#ff3366' : '#00ff88'}`,
          transition: 'left 100ms ease-out',
        }} />
      </div>
      <div style={{
        fontFamily: "'JetBrains Mono', monospace",
        fontSize: '12px',
        color: correlation < threshold ? '#ff3366' : '#00ff88',
        display: 'flex',
        alignItems: 'center',
        gap: '8px',
      }}>
        <span>{correlation.toFixed(2)}</span>
        {autoPhaseActive && correlation < threshold && (
          <span style={{
            background: '#ff660033',
            color: '#ff9933',
            padding: '2px 6px',
            borderRadius: '3px',
            fontSize: '9px',
            animation: 'pulse 1s ease-in-out infinite',
          }}>
            CORRECTING
          </span>
        )}
      </div>
    </div>
  );
};

// Level Meter
const LevelMeter = ({ level, peak, label }) => {
  const segments = 24;
  const activeSegments = Math.floor((level / 100) * segments);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '4px' }}>
      <div style={{
        display: 'flex',
        flexDirection: 'column-reverse',
        gap: '2px',
        height: '120px',
      }}>
        {Array.from({ length: segments }).map((_, i) => {
          const isActive = i < activeSegments;
          let color = '#00ff88';
          if (i >= segments * 0.75) color = '#ff3366';
          else if (i >= segments * 0.6) color = '#ffcc00';

          return (
            <div
              key={i}
              style={{
                width: '8px',
                height: '3px',
                background: isActive ? color : '#222',
                borderRadius: '1px',
                boxShadow: isActive ? `0 0 4px ${color}66` : 'none',
                transition: 'background 50ms',
              }}
            />
          );
        })}
      </div>
      <span style={{
        fontFamily: "'JetBrains Mono', monospace",
        fontSize: '9px',
        color: '#666',
      }}>
        {label}
      </span>
    </div>
  );
};

// Module Icon Components
const HaasIcon = ({ active, color }) => (
  <svg width="60" height="60" viewBox="0 0 60 60" style={{ filter: active ? `drop-shadow(0 0 10px ${color})` : 'none' }}>
    <defs>
      <linearGradient id="haasGrad" x1="0%" y1="0%" x2="100%" y2="0%">
        <stop offset="0%" stopColor={color} stopOpacity="0.5" />
        <stop offset="50%" stopColor={color} />
        <stop offset="100%" stopColor={color} stopOpacity="0.5" />
      </linearGradient>
    </defs>
    {/* Two offset waveforms representing Haas delay */}
    <path
      d="M5 30 Q15 15, 25 30 T45 30 T55 30"
      fill="none"
      stroke={color}
      strokeWidth="2"
      opacity="0.5"
      style={{ animation: active ? 'wave 2s ease-in-out infinite' : 'none' }}
    />
    <path
      d="M10 35 Q20 20, 30 35 T50 35"
      fill="none"
      stroke={color}
      strokeWidth="2.5"
      style={{ animation: active ? 'wave 2s ease-in-out infinite 0.2s' : 'none' }}
    />
  </svg>
);

const WidthIcon = ({ active, color }) => (
  <svg width="60" height="60" viewBox="0 0 60 60" style={{ filter: active ? `drop-shadow(0 0 10px ${color})` : 'none' }}>
    {/* Expanding arrows */}
    <path
      d="M30 30 L10 20 M10 20 L15 25 M10 20 L15 15"
      fill="none"
      stroke={color}
      strokeWidth="2"
      strokeLinecap="round"
      style={{ animation: active ? 'expandLeft 1.5s ease-in-out infinite' : 'none' }}
    />
    <path
      d="M30 30 L50 20 M50 20 L45 25 M50 20 L45 15"
      fill="none"
      stroke={color}
      strokeWidth="2"
      strokeLinecap="round"
      style={{ animation: active ? 'expandRight 1.5s ease-in-out infinite' : 'none' }}
    />
    <path
      d="M30 30 L10 40 M10 40 L15 35 M10 40 L15 45"
      fill="none"
      stroke={color}
      strokeWidth="2"
      strokeLinecap="round"
      style={{ animation: active ? 'expandLeft 1.5s ease-in-out infinite 0.3s' : 'none' }}
    />
    <path
      d="M30 30 L50 40 M50 40 L45 35 M50 40 L45 45"
      fill="none"
      stroke={color}
      strokeWidth="2"
      strokeLinecap="round"
      style={{ animation: active ? 'expandRight 1.5s ease-in-out infinite 0.3s' : 'none' }}
    />
    <circle cx="30" cy="30" r="4" fill={color} />
  </svg>
);

const PhaseIcon = ({ active, correcting, color }) => (
  <svg width="60" height="60" viewBox="0 0 60 60" style={{ filter: active ? `drop-shadow(0 0 10px ${color})` : 'none' }}>
    {/* Phase correlation symbol - two sine waves aligning */}
    <circle cx="30" cy="30" r="20" fill="none" stroke={color} strokeWidth="1.5" opacity="0.3" />
    <circle cx="30" cy="30" r="12" fill="none" stroke={color} strokeWidth="1.5" opacity="0.5" />
    <path
      d="M15 30 Q22 20, 30 30 T45 30"
      fill="none"
      stroke={correcting ? '#ff6600' : color}
      strokeWidth="2"
      style={{ animation: correcting ? 'phaseCorrect 0.5s ease-in-out infinite' : 'none' }}
    />
    <path
      d="M15 30 Q22 40, 30 30 T45 30"
      fill="none"
      stroke={correcting ? '#ff6600' : color}
      strokeWidth="2"
      opacity="0.6"
      style={{ 
        animation: correcting ? 'phaseCorrect 0.5s ease-in-out infinite reverse' : 'none',
        transformOrigin: 'center'
      }}
    />
    {correcting && (
      <text x="30" y="50" textAnchor="middle" fill="#ff6600" fontSize="8" fontFamily="monospace">
        AUTO
      </text>
    )}
  </svg>
);

const OutputIcon = ({ active, color }) => (
  <svg width="60" height="60" viewBox="0 0 60 60" style={{ filter: active ? `drop-shadow(0 0 10px ${color})` : 'none' }}>
    {/* Speaker/output icon */}
    <rect x="18" y="22" width="10" height="16" rx="2" fill={color} opacity="0.8" />
    <path
      d="M28 22 L38 15 L38 45 L28 38 Z"
      fill={color}
    />
    {/* Sound waves */}
    <path d="M42 20 Q50 30, 42 40" fill="none" stroke={color} strokeWidth="2" opacity={active ? 1 : 0.3}
      style={{ animation: active ? 'soundWave 1s ease-out infinite' : 'none' }} />
    <path d="M46 15 Q58 30, 46 45" fill="none" stroke={color} strokeWidth="2" opacity={active ? 0.6 : 0.2}
      style={{ animation: active ? 'soundWave 1s ease-out infinite 0.3s' : 'none' }} />
  </svg>
);

// Main Plugin Component
export default function HaasPhasePlugin() {
  // Plugin state
  const [preset, setPreset] = useState('Clean Stereo');
  const [bypass, setBypass] = useState(false);

  // DELAY section
  const [delayActive, setDelayActive] = useState(true);
  const [leftDelay, setLeftDelay] = useState(0);
  const [rightDelay, setRightDelay] = useState(15);
  const [delayLink, setDelayLink] = useState(false);

  // WIDTH section
  const [widthActive, setWidthActive] = useState(true);
  const [width, setWidth] = useState(150);
  const [lowCut, setLowCut] = useState(250);

  // PHASE section (the star!)
  const [phaseActive, setPhaseActive] = useState(true);
  const [autoPhase, setAutoPhase] = useState(true);
  const [threshold, setThreshold] = useState(0.3);
  const [correctionSpeed, setCorrectionSpeed] = useState(50);
  const [correlation, setCorrelation] = useState(0.65);

  // OUTPUT section
  const [outputActive, setOutputActive] = useState(true);
  const [outputGain, setOutputGain] = useState(0);
  const [dryWet, setDryWet] = useState(100);

  // Simulated levels
  const [inputLevel, setInputLevel] = useState({ l: 0, r: 0 });
  const [outputLevel, setOutputLevel] = useState({ l: 0, r: 0 });

  // Simulate phase correlation changes
  useEffect(() => {
    const interval = setInterval(() => {
      // Simulate correlation based on width and delay
      const baseCorrelation = 1 - (width - 100) / 200 - Math.abs(leftDelay - rightDelay) / 100;
      const noise = (Math.random() - 0.5) * 0.1;
      let newCorrelation = baseCorrelation + noise;

      // If auto phase is on and we're below threshold, gradually improve
      if (autoPhase && phaseActive && newCorrelation < threshold) {
        newCorrelation = Math.min(threshold + 0.05, newCorrelation + 0.02);
      }

      setCorrelation(Math.max(-1, Math.min(1, newCorrelation)));

      // Simulate levels
      const baseLevel = 60 + Math.random() * 20;
      setInputLevel({ l: baseLevel + Math.random() * 10, r: baseLevel + Math.random() * 10 });
      setOutputLevel({
        l: bypass ? 0 : baseLevel * (dryWet / 100) + Math.random() * 10,
        r: bypass ? 0 : baseLevel * (dryWet / 100) + Math.random() * 10
      });
    }, 100);

    return () => clearInterval(interval);
  }, [width, leftDelay, rightDelay, autoPhase, phaseActive, threshold, bypass, dryWet]);

  const presets = ['Clean Stereo', 'Wide Vocals', 'Subtle Width', 'Mono Compatible', 'Extreme Haas'];

  return (
    <div style={{
      width: '1000px',
      height: '625px',
      background: 'linear-gradient(180deg, #1a1a1a 0%, #0d0d0d 100%)',
      borderRadius: '12px',
      fontFamily: "'Inter', -apple-system, sans-serif",
      color: '#fff',
      display: 'flex',
      flexDirection: 'column',
      overflow: 'hidden',
      boxShadow: '0 20px 60px rgba(0,0,0,0.5), inset 0 1px 0 rgba(255,255,255,0.05)',
      border: '1px solid #333',
    }}>
      {/* CSS Animations */}
      <style>{`
        @import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600&family=JetBrains+Mono:wght@400;500&display=swap');

        @keyframes wave {
          0%, 100% { transform: translateY(0); }
          50% { transform: translateY(-3px); }
        }

        @keyframes expandLeft {
          0%, 100% { transform: translateX(0); }
          50% { transform: translateX(-3px); }
        }

        @keyframes expandRight {
          0%, 100% { transform: translateX(0); }
          50% { transform: translateX(3px); }
        }

        @keyframes phaseCorrect {
          0%, 100% { transform: scaleY(1); }
          50% { transform: scaleY(0.8); }
        }

        @keyframes soundWave {
          0% { opacity: 1; transform: translateX(0); }
          100% { opacity: 0; transform: translateX(5px); }
        }

        @keyframes pulse {
          0%, 100% { opacity: 1; }
          50% { opacity: 0.5; }
        }

        @keyframes glow {
          0%, 100% { box-shadow: 0 0 10px currentColor; }
          50% { box-shadow: 0 0 20px currentColor, 0 0 30px currentColor; }
        }
      `}</style>

      {/* Header */}
      <div style={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        padding: '12px 20px',
        borderBottom: '1px solid #2a2a2a',
        background: 'linear-gradient(180deg, #222 0%, #1a1a1a 100%)',
      }}>
        {/* Logo */}
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <svg width="28" height="28" viewBox="0 0 28 28">
            <circle cx="14" cy="14" r="12" fill="none" stroke="#00d4ff" strokeWidth="2" />
            <circle cx="14" cy="14" r="6" fill="#00d4ff" />
            <path d="M14 2 L14 8 M14 20 L14 26 M2 14 L8 14 M20 14 L26 14" stroke="#00d4ff" strokeWidth="2" />
          </svg>
          <span style={{
            fontSize: '18px',
            fontWeight: '600',
            letterSpacing: '2px',
            background: 'linear-gradient(90deg, #00d4ff, #00ff88)',
            WebkitBackgroundClip: 'text',
            WebkitTextFillColor: 'transparent',
          }}>
            HAAS<span style={{ opacity: 0.6 }}>FX</span>
          </span>
          <span style={{
            fontSize: '10px',
            background: '#00d4ff22',
            color: '#00d4ff',
            padding: '2px 6px',
            borderRadius: '3px',
            marginLeft: '4px',
          }}>
            PRO
          </span>
        </div>

        {/* Preset Selector */}
        <div style={{
          display: 'flex',
          alignItems: 'center',
          gap: '8px',
        }}>
          <button style={{
            background: 'none',
            border: 'none',
            color: '#666',
            cursor: 'pointer',
            padding: '4px',
          }}>
            ←
          </button>
          <div style={{
            background: '#0d0d0d',
            padding: '8px 24px',
            borderRadius: '6px',
            border: '1px solid #333',
            minWidth: '180px',
            textAlign: 'center',
            fontSize: '13px',
            color: '#ccc',
          }}>
            {preset}
          </div>
          <button style={{
            background: 'none',
            border: 'none',
            color: '#666',
            cursor: 'pointer',
            padding: '4px',
          }}>
            →
          </button>
        </div>

        {/* Bypass */}
        <button
          onClick={() => setBypass(!bypass)}
          style={{
            background: bypass ? '#ff3366' : '#2a2a2a',
            border: '1px solid',
            borderColor: bypass ? '#ff3366' : '#444',
            color: bypass ? '#fff' : '#888',
            padding: '8px 20px',
            borderRadius: '6px',
            cursor: 'pointer',
            fontSize: '12px',
            fontWeight: '600',
            letterSpacing: '1px',
            transition: 'all 150ms',
          }}
        >
          BYPASS
        </button>
      </div>

      {/* Main Content */}
      <div style={{
        display: 'flex',
        flex: 1,
        padding: '15px',
        gap: '15px',
      }}>
        {/* INPUT Meters - Left Side */}
        <div style={{
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'center',
          padding: '10px 8px',
          background: '#0d0d0d',
          borderRadius: '8px',
          border: '1px solid #2a2a2a',
          gap: '8px',
        }}>
          <span style={{
            fontSize: '9px',
            color: '#666',
            letterSpacing: '1px',
            writingMode: 'vertical-rl',
            transform: 'rotate(180deg)',
          }}>
            INPUT
          </span>
          <div style={{ display: 'flex', gap: '4px' }}>
            <LevelMeter level={inputLevel.l} label="L" />
            <LevelMeter level={inputLevel.r} label="R" />
          </div>
          <div style={{
            fontFamily: "'JetBrains Mono', monospace",
            fontSize: '9px',
            color: '#00ff88',
            marginTop: '4px',
          }}>
            0.0dB
          </div>
        </div>

        {/* DELAY Module */}
        <div style={{
          flex: 1,
          background: '#141414',
          borderRadius: '10px',
          border: '1px solid #2a2a2a',
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
        }}>
          {/* Module Header */}
          <div style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            padding: '10px 15px',
            borderBottom: '1px solid #2a2a2a',
          }}>
            <button
              onClick={() => setDelayActive(!delayActive)}
              style={{
                width: '20px',
                height: '20px',
                borderRadius: '50%',
                border: '2px solid',
                borderColor: delayActive ? '#ff66aa' : '#444',
                background: delayActive ? '#ff66aa33' : 'transparent',
                cursor: 'pointer',
              }}
            />
            <span style={{
              color: '#ff66aa',
              fontSize: '13px',
              fontWeight: '600',
              letterSpacing: '2px',
            }}>
              DELAY
            </span>
            <div style={{ width: '20px' }} />
          </div>

          {/* Icon */}
          <div style={{
            display: 'flex',
            justifyContent: 'center',
            padding: '15px 0',
            opacity: delayActive ? 1 : 0.3,
          }}>
            <HaasIcon active={delayActive} color="#ff66aa" />
          </div>

          {/* Controls */}
          <div style={{
            flex: 1,
            padding: '15px',
            display: 'flex',
            flexDirection: 'column',
            gap: '20px',
            opacity: delayActive ? 1 : 0.4,
          }}>
            <div style={{ display: 'flex', justifyContent: 'space-around' }}>
              <div style={{ textAlign: 'center' }}>
                <VerticalSlider
                  value={leftDelay}
                  onChange={setLeftDelay}
                  min={0}
                  max={50}
                  height={100}
                  color="#ff66aa"
                />
                <div style={{
                  marginTop: '8px',
                  fontFamily: "'JetBrains Mono', monospace",
                  fontSize: '11px',
                  color: '#ff66aa',
                }}>
                  {leftDelay.toFixed(1)}ms
                </div>
                <div style={{ fontSize: '10px', color: '#666', marginTop: '2px' }}>LEFT</div>
              </div>
              <div style={{ textAlign: 'center' }}>
                <VerticalSlider
                  value={rightDelay}
                  onChange={setRightDelay}
                  min={0}
                  max={50}
                  height={100}
                  color="#ff66aa"
                />
                <div style={{
                  marginTop: '8px',
                  fontFamily: "'JetBrains Mono', monospace",
                  fontSize: '11px',
                  color: '#ff66aa',
                }}>
                  {rightDelay.toFixed(1)}ms
                </div>
                <div style={{ fontSize: '10px', color: '#666', marginTop: '2px' }}>RIGHT</div>
              </div>
            </div>

            <button
              onClick={() => setDelayLink(!delayLink)}
              style={{
                alignSelf: 'center',
                background: delayLink ? '#ff66aa33' : '#1a1a1a',
                border: '1px solid',
                borderColor: delayLink ? '#ff66aa' : '#333',
                color: delayLink ? '#ff66aa' : '#666',
                padding: '6px 16px',
                borderRadius: '4px',
                fontSize: '10px',
                cursor: 'pointer',
                letterSpacing: '1px',
              }}
            >
              LINK
            </button>
          </div>
        </div>

        {/* WIDTH Module */}
        <div style={{
          flex: 1,
          background: '#141414',
          borderRadius: '10px',
          border: '1px solid #2a2a2a',
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
        }}>
          <div style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            padding: '10px 15px',
            borderBottom: '1px solid #2a2a2a',
          }}>
            <button
              onClick={() => setWidthActive(!widthActive)}
              style={{
                width: '20px',
                height: '20px',
                borderRadius: '50%',
                border: '2px solid',
                borderColor: widthActive ? '#00d4ff' : '#444',
                background: widthActive ? '#00d4ff33' : 'transparent',
                cursor: 'pointer',
              }}
            />
            <span style={{
              color: '#00d4ff',
              fontSize: '13px',
              fontWeight: '600',
              letterSpacing: '2px',
            }}>
              WIDTH
            </span>
            <div style={{ width: '20px' }} />
          </div>

          <div style={{
            display: 'flex',
            justifyContent: 'center',
            padding: '15px 0',
            opacity: widthActive ? 1 : 0.3,
          }}>
            <WidthIcon active={widthActive} color="#00d4ff" />
          </div>

          <div style={{
            flex: 1,
            padding: '15px',
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            gap: '25px',
            opacity: widthActive ? 1 : 0.4,
          }}>
            <Knob
              value={width}
              onChange={setWidth}
              min={100}
              max={200}
              size={90}
              color="#00d4ff"
              unit="%"
              label="STEREO WIDTH"
            />

            <div style={{ width: '100%' }}>
              <div style={{
                display: 'flex',
                justifyContent: 'space-between',
                fontSize: '10px',
                color: '#666',
                marginBottom: '4px',
              }}>
                <span>LOW CUT</span>
                <span style={{ color: '#00d4ff' }}>{lowCut}Hz</span>
              </div>
              <input
                type="range"
                min={20}
                max={500}
                value={lowCut}
                onChange={(e) => setLowCut(Number(e.target.value))}
                style={{
                  width: '100%',
                  accentColor: '#00d4ff',
                }}
              />
            </div>
          </div>
        </div>

        {/* PHASE Module - THE STAR! */}
        <div style={{
          flex: 1.3,
          background: autoPhase && correlation < threshold ? '#1a1410' : '#141414',
          borderRadius: '10px',
          border: '1px solid',
          borderColor: autoPhase && correlation < threshold ? '#ff660066' : '#2a2a2a',
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
          transition: 'all 300ms',
          boxShadow: autoPhase && correlation < threshold ? '0 0 30px #ff660022' : 'none',
        }}>
          <div style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            padding: '10px 15px',
            borderBottom: '1px solid #2a2a2a',
          }}>
            <button
              onClick={() => setPhaseActive(!phaseActive)}
              style={{
                width: '20px',
                height: '20px',
                borderRadius: '50%',
                border: '2px solid',
                borderColor: phaseActive ? '#ff9933' : '#444',
                background: phaseActive ? '#ff993333' : 'transparent',
                cursor: 'pointer',
              }}
            />
            <span style={{
              color: '#ff9933',
              fontSize: '13px',
              fontWeight: '600',
              letterSpacing: '2px',
            }}>
              AUTO PHASE
            </span>
            <span style={{
              fontSize: '9px',
              background: '#ff993322',
              color: '#ff9933',
              padding: '2px 6px',
              borderRadius: '3px',
            }}>
              EXCLUSIVE
            </span>
          </div>

          <div style={{
            display: 'flex',
            justifyContent: 'center',
            padding: '15px 0',
            opacity: phaseActive ? 1 : 0.3,
          }}>
            <PhaseIcon
              active={phaseActive}
              correcting={autoPhase && phaseActive && correlation < threshold}
              color="#ff9933"
            />
          </div>

          <div style={{
            flex: 1,
            padding: '15px',
            display: 'flex',
            flexDirection: 'column',
            gap: '20px',
            opacity: phaseActive ? 1 : 0.4,
          }}>
            {/* Phase Correlation Meter */}
            <PhaseCorrelationMeter
              correlation={correlation}
              autoPhaseActive={autoPhase && phaseActive}
              threshold={threshold}
            />

            {/* Auto Phase Toggle */}
            <button
              onClick={() => setAutoPhase(!autoPhase)}
              style={{
                alignSelf: 'center',
                background: autoPhase ? 'linear-gradient(180deg, #ff9933, #ff6600)' : '#1a1a1a',
                border: '1px solid',
                borderColor: autoPhase ? '#ff9933' : '#333',
                color: autoPhase ? '#000' : '#666',
                padding: '10px 24px',
                borderRadius: '6px',
                fontSize: '11px',
                fontWeight: '600',
                cursor: 'pointer',
                letterSpacing: '1px',
                boxShadow: autoPhase ? '0 0 20px #ff993366' : 'none',
                transition: 'all 200ms',
              }}
            >
              AUTO FIX {autoPhase ? 'ON' : 'OFF'}
            </button>

            {/* Controls Row */}
            <div style={{ display: 'flex', justifyContent: 'space-around' }}>
              <div style={{ textAlign: 'center' }}>
                <Knob
                  value={threshold}
                  onChange={setThreshold}
                  min={0}
                  max={1}
                  size={60}
                  color="#ff9933"
                  showValue={false}
                />
                <div style={{
                  fontFamily: "'JetBrains Mono', monospace",
                  fontSize: '11px',
                  color: '#ff9933',
                  marginTop: '4px',
                }}>
                  {threshold.toFixed(2)}
                </div>
                <div style={{ fontSize: '9px', color: '#666', marginTop: '2px' }}>THRESHOLD</div>
              </div>

              <div style={{ textAlign: 'center' }}>
                <Knob
                  value={correctionSpeed}
                  onChange={setCorrectionSpeed}
                  min={0}
                  max={100}
                  size={60}
                  color="#ff9933"
                  showValue={false}
                />
                <div style={{
                  fontFamily: "'JetBrains Mono', monospace",
                  fontSize: '11px',
                  color: '#ff9933',
                  marginTop: '4px',
                }}>
                  {correctionSpeed.toFixed(0)}%
                </div>
                <div style={{ fontSize: '9px', color: '#666', marginTop: '2px' }}>SPEED</div>
              </div>
            </div>
          </div>
        </div>

        {/* OUTPUT Module */}
        <div style={{
          flex: 1,
          background: '#141414',
          borderRadius: '10px',
          border: '1px solid #2a2a2a',
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
        }}>
          <div style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            padding: '10px 15px',
            borderBottom: '1px solid #2a2a2a',
          }}>
            <button
              onClick={() => setOutputActive(!outputActive)}
              style={{
                width: '20px',
                height: '20px',
                borderRadius: '50%',
                border: '2px solid',
                borderColor: outputActive ? '#00ff88' : '#444',
                background: outputActive ? '#00ff8833' : 'transparent',
                cursor: 'pointer',
              }}
            />
            <span style={{
              color: '#00ff88',
              fontSize: '13px',
              fontWeight: '600',
              letterSpacing: '2px',
            }}>
              OUTPUT
            </span>
            <div style={{ width: '20px' }} />
          </div>

          <div style={{
            display: 'flex',
            justifyContent: 'center',
            padding: '15px 0',
            opacity: outputActive ? 1 : 0.3,
          }}>
            <OutputIcon active={outputActive && !bypass} color="#00ff88" />
          </div>

          <div style={{
            flex: 1,
            padding: '15px',
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            gap: '20px',
            opacity: outputActive ? 1 : 0.4,
          }}>
            <Knob
              value={outputGain}
              onChange={setOutputGain}
              min={-12}
              max={12}
              size={80}
              color="#00ff88"
              unit="dB"
              label="GAIN"
            />

            <div style={{ width: '100%' }}>
              <div style={{
                display: 'flex',
                justifyContent: 'space-between',
                fontSize: '10px',
                color: '#666',
                marginBottom: '4px',
              }}>
                <span>DRY/WET</span>
                <span style={{ color: '#00ff88' }}>{dryWet}%</span>
              </div>
              <input
                type="range"
                min={0}
                max={100}
                value={dryWet}
                onChange={(e) => setDryWet(Number(e.target.value))}
                style={{
                  width: '100%',
                  accentColor: '#00ff88',
                }}
              />
            </div>
          </div>
        </div>

        {/* OUTPUT Meters - Right Side */}
        <div style={{
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'center',
          padding: '10px 8px',
          background: '#0d0d0d',
          borderRadius: '8px',
          border: '1px solid #2a2a2a',
          gap: '8px',
        }}>
          <span style={{
            fontSize: '9px',
            color: '#666',
            letterSpacing: '1px',
            writingMode: 'vertical-rl',
            transform: 'rotate(180deg)',
          }}>
            OUTPUT
          </span>
          <div style={{ display: 'flex', gap: '4px' }}>
            <LevelMeter level={outputLevel.l} label="L" />
            <LevelMeter level={outputLevel.r} label="R" />
          </div>
          <div style={{
            fontFamily: "'JetBrains Mono', monospace",
            fontSize: '9px',
            color: '#00ff88',
            marginTop: '4px',
          }}>
            {outputGain >= 0 ? '+' : ''}{outputGain.toFixed(1)}dB
          </div>
        </div>
      </div>

      {/* Footer */}
      <div style={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        padding: '10px 20px',
        borderTop: '1px solid #2a2a2a',
        background: '#0d0d0d',
      }}>
        {/* Center Logo */}
        <div style={{
          display: 'flex',
          alignItems: 'center',
          gap: '6px',
          color: '#444',
          fontSize: '11px',
          letterSpacing: '2px',
        }}>
          <span>REORDER</span>
          <span style={{ color: '#00d4ff' }}>AUDIO</span>
        </div>
      </div>
    </div>
  );
}
