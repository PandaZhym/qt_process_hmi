#ifndef SVG_ASSETS_H
#define SVG_ASSETS_H

namespace svg_assets {

// 罐体 — 工业钢罐，观察窗 + 铆钉
constexpr const char *TANK_BODY = R"SVG_(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 200 340">
  <defs>
    <linearGradient id="tankBody" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#5a6070"/>
      <stop offset="15%" stop-color="#7a8294"/>
      <stop offset="30%" stop-color="#8a92a4"/>
      <stop offset="50%" stop-color="#6a7282"/>
      <stop offset="70%" stop-color="#5a6272"/>
      <stop offset="85%" stop-color="#4a5260"/>
      <stop offset="100%" stop-color="#3a4250"/>
    </linearGradient>
    <linearGradient id="tankView" x1="0" y1="0" x2="1" y2="1">
      <stop offset="0%" stop-color="#1a1d25" stop-opacity="0.9"/>
      <stop offset="50%" stop-color="#14171e" stop-opacity="0.95"/>
      <stop offset="100%" stop-color="#1a1d25" stop-opacity="0.9"/>
    </linearGradient>
    <linearGradient id="rivetGrad" x1="0" y1="0" x2="1" y2="1">
      <stop offset="0%" stop-color="#c0c5cc"/>
      <stop offset="100%" stop-color="#606570"/>
    </linearGradient>
  </defs>
  <!-- 罐体外框 -->
  <rect x="10" y="10" width="180" height="320" rx="28" fill="url(#tankBody)" stroke="#3a4050" stroke-width="2"/>
  <!-- 顶部法兰 -->
  <rect x="40" y="2" width="120" height="18" rx="6" fill="#6a7282" stroke="#3a4050" stroke-width="1.5"/>
  <rect x="60" y="0" width="80" height="6" rx="3" fill="#8a92a4"/>
  <!-- 底部排出管 -->
  <rect x="72" y="326" width="56" height="14" rx="5" fill="#6a7282" stroke="#3a4050" stroke-width="1"/>
  <!-- 观察窗（竖条） -->
  <rect x="28" y="40" width="18" height="250" rx="9" fill="url(#tankView)" stroke="#4a5260" stroke-width="1.5"/>
  <!-- 观察窗刻度标记 -->
  <line x1="34" y1="70" x2="40" y2="70" stroke="#60a0c0" stroke-width="1" opacity="0.7"/>
  <line x1="34" y1="120" x2="40" y2="120" stroke="#60a0c0" stroke-width="1" opacity="0.7"/>
  <line x1="34" y1="170" x2="40" y2="170" stroke="#60a0c0" stroke-width="1" opacity="0.7"/>
  <line x1="34" y1="220" x2="40" y2="220" stroke="#60a0c0" stroke-width="1" opacity="0.7"/>
  <line x1="34" y1="270" x2="40" y2="270" stroke="#60a0c0" stroke-width="1" opacity="0.7"/>
  <!-- 左侧铆钉列 -->
  <circle cx="22" cy="40" r="4" fill="url(#rivetGrad)"/>
  <circle cx="22" cy="100" r="4" fill="url(#rivetGrad)"/>
  <circle cx="22" cy="160" r="4" fill="url(#rivetGrad)"/>
  <circle cx="22" cy="220" r="4" fill="url(#rivetGrad)"/>
  <circle cx="22" cy="280" r="4" fill="url(#rivetGrad)"/>
  <!-- 右侧铆钉列 -->
  <circle cx="178" cy="40" r="4" fill="url(#rivetGrad)"/>
  <circle cx="178" cy="100" r="4" fill="url(#rivetGrad)"/>
  <circle cx="178" cy="160" r="4" fill="url(#rivetGrad)"/>
  <circle cx="178" cy="220" r="4" fill="url(#rivetGrad)"/>
  <circle cx="178" cy="280" r="4" fill="url(#rivetGrad)"/>
  <!-- 底部支撑座 -->
  <rect x="30" y="324" width="140" height="8" rx="4" fill="#5a6272" stroke="#3a4050" stroke-width="1"/>
  <rect x="50" y="330" width="100" height="8" rx="3" fill="#4a5260" stroke="#3a4050" stroke-width="1"/>
</svg>
)SVG_";

// 泵体 — 卧式离心泵
constexpr const char *PUMP_BODY = R"SVG_(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 180 160">
  <defs>
    <radialGradient id="pumpVolute" cx="0.45" cy="0.4">
      <stop offset="0%" stop-color="#9aa2b0"/>
      <stop offset="40%" stop-color="#7a8294"/>
      <stop offset="80%" stop-color="#5a6272"/>
      <stop offset="100%" stop-color="#3a4250"/>
    </radialGradient>
    <linearGradient id="flangeGrad" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#7a8294"/>
      <stop offset="50%" stop-color="#5a6272"/>
      <stop offset="100%" stop-color="#3a4250"/>
    </linearGradient>
    <linearGradient id="motorGrad" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#8a92a4"/>
      <stop offset="30%" stop-color="#6a7282"/>
      <stop offset="100%" stop-color="#3a4250"/>
    </linearGradient>
    <radialGradient id="hubGrad" cx="0.4" cy="0.35">
      <stop offset="0%" stop-color="#d0d5dc"/>
      <stop offset="100%" stop-color="#707880"/>
    </radialGradient>
  </defs>
  <!-- 底座 -->
  <rect x="10" y="130" width="160" height="18" rx="4" fill="#5a6272" stroke="#3a4050" stroke-width="1.5"/>
  <rect x="20" y="144" width="140" height="8" rx="2" fill="#4a5260" stroke="#3a4050" stroke-width="1"/>
  <!-- 泵体（蜗壳） -->
  <ellipse cx="90" cy="70" rx="55" ry="52" fill="url(#pumpVolute)" stroke="#3a4050" stroke-width="2"/>
  <!-- 进口法兰（左） -->
  <rect x="30" y="75" width="22" height="24" rx="4" fill="url(#flangeGrad)" stroke="#3a4050" stroke-width="1.5"/>
  <rect x="25" y="78" width="8" height="18" rx="2" fill="#5a6272"/>
  <!-- 出口法兰（上） -->
  <rect x="72" y="12" width="24" height="22" rx="4" fill="url(#flangeGrad)" stroke="#3a4050" stroke-width="1.5"/>
  <rect x="75" y="6" width="18" height="8" rx="2" fill="#5a6272"/>
  <!-- 电机壳（右侧） -->
  <ellipse cx="90" cy="70" rx="38" ry="36" fill="url(#motorGrad)" stroke="#4a5260" stroke-width="1.5"/>
  <!-- 散热筋 -->
  <line x1="60" y1="52" x2="60" y2="88" stroke="#5a6272" stroke-width="2"/>
  <line x1="68" y1="48" x2="68" y2="92" stroke="#5a6272" stroke-width="2"/>
  <line x1="76" y1="46" x2="76" y2="94" stroke="#5a6272" stroke-width="2"/>
  <line x1="84" y1="46" x2="84" y2="94" stroke="#5a6272" stroke-width="2"/>
  <line x1="92" y1="46" x2="92" y2="94" stroke="#5a6272" stroke-width="2"/>
  <line x1="100" y1="48" x2="100" y2="92" stroke="#5a6272" stroke-width="2"/>
  <line x1="108" y1="52" x2="108" y2="88" stroke="#5a6272" stroke-width="2"/>
  <!-- 叶轮室视窗 -->
  <ellipse cx="90" cy="70" rx="20" ry="20" fill="none" stroke="#4a5260" stroke-width="2" opacity="0.6"/>
  <!-- 铭牌 -->
  <rect x="110" y="100" width="40" height="22" rx="3" fill="#d8dce0" stroke="#a0a8b0" stroke-width="1"/>
  <line x1="112" y1="107" x2="148" y2="107" stroke="#606570" stroke-width="0.8"/>
  <line x1="112" y1="112" x2="148" y2="112" stroke="#606570" stroke-width="0.8"/>
  <line x1="112" y1="117" x2="140" y2="117" stroke="#606570" stroke-width="0.8"/>
</svg>
)SVG_";

// 阀门 — 截止阀，手轮 + 阀杆
constexpr const char *VALVE_BODY = R"SVG_(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 140 160">
  <defs>
    <linearGradient id="valveBodyGrad" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#8a92a4"/>
      <stop offset="40%" stop-color="#6a7282"/>
      <stop offset="100%" stop-color="#4a5260"/>
    </linearGradient>
    <linearGradient id="bonnetGrad" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#7a8294"/>
      <stop offset="100%" stop-color="#4a5260"/>
    </linearGradient>
    <linearGradient id="handwheelGrad" x1="0" y1="0" x2="1" y2="1">
      <stop offset="0%" stop-color="#d0d5dc"/>
      <stop offset="100%" stop-color="#707880"/>
    </linearGradient>
    <radialGradient id="stemGrad" cx="0.4" cy="0.3">
      <stop offset="0%" stop-color="#c0c5cc"/>
      <stop offset="100%" stop-color="#606570"/>
    </radialGradient>
  </defs>
  <!-- 阀体 -->
  <ellipse cx="70" cy="100" rx="44" ry="30" fill="url(#valveBodyGrad)" stroke="#3a4050" stroke-width="2"/>
  <!-- 左法兰 -->
  <rect x="20" y="88" width="20" height="24" rx="4" fill="#6a7282" stroke="#3a4050" stroke-width="1.5"/>
  <circle cx="32" cy="94" r="2.5" fill="#9098a5"/>
  <circle cx="32" cy="106" r="2.5" fill="#9098a5"/>
  <!-- 右法兰 -->
  <rect x="100" y="88" width="20" height="24" rx="4" fill="#6a7282" stroke="#3a4050" stroke-width="1.5"/>
  <circle cx="108" cy="94" r="2.5" fill="#9098a5"/>
  <circle cx="108" cy="106" r="2.5" fill="#9098a5"/>
  <!-- 阀盖 (bonnet) -->
  <rect x="46" y="62" width="48" height="30" rx="6" fill="url(#bonnetGrad)" stroke="#3a4050" stroke-width="1.5"/>
  <!-- 填料函 -->
  <rect x="54" y="52" width="32" height="14" rx="3" fill="#5a6272" stroke="#3a4050" stroke-width="1"/>
  <rect x="56" y="48" width="28" height="6" rx="2" fill="#6a7282" stroke="#3a4050" stroke-width="1"/>
  <!-- 阀杆 -->
  <rect x="66" y="16" width="8" height="36" rx="3" fill="url(#stemGrad)" stroke="#3a4050" stroke-width="1"/>
  <!-- 手轮 -->
  <ellipse cx="70" cy="18" rx="30" ry="10" fill="none" stroke="#9098a5" stroke-width="3.5"/>
  <line x1="70" y1="18" x2="70" y2="26" stroke="#7a8294" stroke-width="3"/>
  <!-- 手轮辐条 -->
  <line x1="45" y1="16" x2="56" y2="22" stroke="#a0a8b5" stroke-width="2.5"/>
  <line x1="95" y1="16" x2="84" y2="22" stroke="#a0a8b5" stroke-width="2.5"/>
  <circle cx="70" cy="18" r="5" fill="url(#stemGrad)" stroke="#3a4050" stroke-width="1"/>
</svg>
)SVG_";

// 管道 — 带焊缝的工业管道段
constexpr const char *PIPE_HORIZ = R"SVG_(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 300 40">
  <defs>
    <linearGradient id="pipeBody" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#4a5260"/>
      <stop offset="18%" stop-color="#6a7282"/>
      <stop offset="40%" stop-color="#8a92a4"/>
      <stop offset="65%" stop-color="#6a7282"/>
      <stop offset="85%" stop-color="#5a6272"/>
      <stop offset="100%" stop-color="#3a4250"/>
    </linearGradient>
    <linearGradient id="flangeGrad" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#9098a5"/>
      <stop offset="50%" stop-color="#6a7282"/>
      <stop offset="100%" stop-color="#4a5260"/>
    </linearGradient>
  </defs>
  <!-- 管体 -->
  <rect x="16" y="10" width="268" height="20" rx="6" fill="url(#pipeBody)" stroke="#3a4050" stroke-width="1"/>
  <!-- 顶部高光 -->
  <rect x="18" y="12" width="264" height="6" rx="3" fill="white" opacity="0.08"/>
  <!-- 左法兰 -->
  <rect x="4" y="4" width="16" height="32" rx="4" fill="url(#flangeGrad)" stroke="#3a4050" stroke-width="1.5"/>
  <!-- 右法兰 -->
  <rect x="280" y="4" width="16" height="32" rx="4" fill="url(#flangeGrad)" stroke="#3a4050" stroke-width="1.5"/>
  <!-- 焊缝标记 -->
  <rect x="10" y="18" width="4" height="4" rx="1" fill="#a0a8b5" opacity="0.5"/>
  <rect x="286" y="18" width="4" height="4" rx="1" fill="#a0a8b5" opacity="0.5"/>
</svg>
)SVG_";

constexpr const char *PIPE_VERT = R"SVG_(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 40 300">
  <defs>
    <linearGradient id="pipeBodyV" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#4a5260"/>
      <stop offset="18%" stop-color="#6a7282"/>
      <stop offset="40%" stop-color="#8a92a4"/>
      <stop offset="65%" stop-color="#6a7282"/>
      <stop offset="85%" stop-color="#5a6272"/>
      <stop offset="100%" stop-color="#3a4250"/>
    </linearGradient>
    <linearGradient id="flangeGradV" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#9098a5"/>
      <stop offset="50%" stop-color="#6a7282"/>
      <stop offset="100%" stop-color="#4a5260"/>
    </linearGradient>
  </defs>
  <rect x="10" y="16" width="20" height="268" rx="6" fill="url(#pipeBodyV)" stroke="#3a4050" stroke-width="1"/>
  <rect x="12" y="18" width="6" height="264" rx="3" fill="white" opacity="0.08"/>
  <rect x="4" y="4" width="32" height="16" rx="4" fill="url(#flangeGradV)" stroke="#3a4050" stroke-width="1.5"/>
  <rect x="4" y="280" width="32" height="16" rx="4" fill="url(#flangeGradV)" stroke="#3a4050" stroke-width="1.5"/>
  <rect x="18" y="10" width="4" height="4" rx="1" fill="#a0a8b5" opacity="0.5"/>
  <rect x="18" y="286" width="4" height="4" rx="1" fill="#a0a8b5" opacity="0.5"/>
</svg>
)SVG_";

// 仪表面板 — 数值显示器外壳
constexpr const char *VALUE_PANEL = R"SVG_(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 200 120">
  <defs>
    <linearGradient id="panelBody" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#5a6272"/>
      <stop offset="50%" stop-color="#4a5260"/>
      <stop offset="100%" stop-color="#3a4250"/>
    </linearGradient>
    <linearGradient id="bezelGrad" x1="0" y1="0" x2="1" y2="1">
      <stop offset="0%" stop-color="#a0a8b5"/>
      <stop offset="30%" stop-color="#7a8294"/>
      <stop offset="70%" stop-color="#5a6272"/>
      <stop offset="100%" stop-color="#3a4250"/>
    </linearGradient>
    <linearGradient id="lcdGrad" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#242a32"/>
      <stop offset="50%" stop-color="#1e232a"/>
      <stop offset="100%" stop-color="#1a1f26"/>
    </linearGradient>
  </defs>
  <!-- 外边 -->
  <rect x="2" y="2" width="196" height="116" rx="10" fill="url(#panelBody)" stroke="#2a3038" stroke-width="1.5"/>
  <!-- 斜角边框（内） -->
  <rect x="6" y="6" width="188" height="108" rx="8" fill="url(#bezelGrad)" stroke="#3a4050" stroke-width="1"/>
  <!-- LCD 面板 -->
  <rect x="12" y="12" width="176" height="96" rx="6" fill="url(#lcdGrad)" stroke="#1a1f26" stroke-width="1.5"/>
  <!-- 玻璃反光 -->
  <rect x="14" y="14" width="172" height="40" rx="4" fill="white" opacity="0.03"/>
</svg>
)SVG_";

} // namespace svg_assets
#endif
