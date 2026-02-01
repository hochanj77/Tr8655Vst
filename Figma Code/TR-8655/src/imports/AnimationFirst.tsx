import imgAnimationFirst from "figma:asset/4afd3b3452e1b94a144686d0fb0bac3550677f44.png";
import imgAnimationFirst1 from "figma:asset/9616b57a0a1bff7cab9564ffce393096c5c4dfa2.png";

export default function AnimationFirst({ className }: { className?: string }) {
  return (
    <div className={className || "relative size-[276px]"} data-name="Animation/First">
      <div className="absolute inset-[-7.25%]">
        <img alt="" className="block max-w-none size-full" height="316" src={imgAnimationFirst} width="316" />
      </div>
      <div className="absolute inset-[-1.81%]">
        <img alt="" className="block max-w-none size-full" height="286" src={imgAnimationFirst1} width="286" />
      </div>
      <svg className="block size-full" fill="none" preserveAspectRatio="none" viewBox="0 0 276 276">
        <g filter="url(#filter0_iii_1_31)" id="Ellipse 34">
          <circle cx="138" cy="138" fill="var(--fill-0, #080722)" r="138" />
          <circle cx="138" cy="138" fill="url(#paint0_radial_1_31)" fillOpacity="0.9" r="138" />
          <circle cx="138" cy="138" fill="url(#paint1_radial_1_31)" fillOpacity="0.85" r="138" />
          <circle cx="138" cy="138" fill="url(#paint2_radial_1_31)" fillOpacity="0.8" r="138" />
          <circle cx="138" cy="138" fill="url(#paint3_radial_1_31)" fillOpacity="0.45" r="138" />
        </g>
        <defs>
          <filter colorInterpolationFilters="sRGB" filterUnits="userSpaceOnUse" height="276" id="filter0_iii_1_31" width="276" x="0" y="0">
            <feFlood floodOpacity="0" result="BackgroundImageFix" />
            <feBlend in="SourceGraphic" in2="BackgroundImageFix" mode="normal" result="shape" />
            <feColorMatrix in="SourceAlpha" result="hardAlpha" type="matrix" values="0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 127 0" />
            <feOffset />
            <feGaussianBlur stdDeviation="20" />
            <feComposite in2="hardAlpha" k2="-1" k3="1" operator="arithmetic" />
            <feColorMatrix type="matrix" values="0 0 0 0 0.185021 0 0 0 0 0.156042 0 0 0 0 0.735625 0 0 0 1 0" />
            <feBlend in2="shape" mode="normal" result="effect1_innerShadow_1_31" />
            <feColorMatrix in="SourceAlpha" result="hardAlpha" type="matrix" values="0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 127 0" />
            <feOffset />
            <feGaussianBlur stdDeviation="15" />
            <feComposite in2="hardAlpha" k2="-1" k3="1" operator="arithmetic" />
            <feColorMatrix type="matrix" values="0 0 0 0 0.2075 0 0 0 0 0.175 0 0 0 0 0.825 0 0 0 1 0" />
            <feBlend in2="effect1_innerShadow_1_31" mode="normal" result="effect2_innerShadow_1_31" />
            <feColorMatrix in="SourceAlpha" result="hardAlpha" type="matrix" values="0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 127 0" />
            <feOffset />
            <feGaussianBlur stdDeviation="6" />
            <feComposite in2="hardAlpha" k2="-1" k3="1" operator="arithmetic" />
            <feColorMatrix type="matrix" values="0 0 0 0 0.791969 0 0 0 0 0.783437 0 0 0 0 0.954063 0 0 0 1 0" />
            <feBlend in2="effect2_innerShadow_1_31" mode="normal" result="effect3_innerShadow_1_31" />
          </filter>
          <radialGradient cx="0" cy="0" gradientTransform="translate(80.04 17.94) rotate(63.4457) scale(134.219 159.219)" gradientUnits="userSpaceOnUse" id="paint0_radial_1_31" r="1">
            <stop stopColor="#8F8AE6" stopOpacity="0.63" />
            <stop offset="1" stopColor="#2C25AD" stopOpacity="0" />
          </radialGradient>
          <radialGradient cx="0" cy="0" gradientTransform="translate(268.652 93.4977) rotate(151.458) scale(122.021)" gradientUnits="userSpaceOnUse" id="paint1_radial_1_31" r="1">
            <stop stopColor="#3E36D4" />
            <stop offset="1" stopColor="#3029C0" stopOpacity="0" />
          </radialGradient>
          <radialGradient cx="0" cy="0" gradientTransform="translate(183.728 268.652) rotate(-108.939) scale(88.0572)" gradientUnits="userSpaceOnUse" id="paint2_radial_1_31" r="1">
            <stop stopColor="#5B54DB" />
            <stop offset="1" stopColor="#5B55DB" stopOpacity="0" />
          </radialGradient>
          <radialGradient cx="0" cy="0" gradientTransform="translate(106.26 138) rotate(-19.7672) scale(93.8501 139.854)" gradientUnits="userSpaceOnUse" id="paint3_radial_1_31" r="1">
            <stop stopColor="#2D26B2" />
            <stop offset="1" stopColor="#453DD6" stopOpacity="0" />
          </radialGradient>
        </defs>
      </svg>
    </div>
  );
}