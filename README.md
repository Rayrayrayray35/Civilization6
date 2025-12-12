文明风格六边形地图系统使用指南
文件结构
Source/YourProject/
├── CiviTypes.h              # 枚举定义 (ETerrain, ELandform)
├── Landblock.h/.cpp         # 地块类
├── Civi_GameModeBase.h/.cpp # 游戏模式（地图生成）
├── TerrainDataAsset.h/.cpp  # 地形数据资产
└── HexMapRenderer.h/.cpp    # 地图渲染器
使用步骤
1. 替换API宏
在所有 .h 文件中，将 CIVIGAME_API 替换为你的项目API宏。
例如你的项目叫 MyGame，则改为 MYGAME_API。
2. 创建六边形网格模型
你需要一个六边形静态网格：

在 Blender 中创建一个正六边形平面
外接圆半径建议 100 单位
导出为 FBX 导入 UE

或者使用 UE 的 ProceduralMesh 在运行时生成。
3. 创建 TerrainDataAsset

在 Content Browser 中右键
选择 Miscellaneous → Data Asset
选择 TerrainDataAsset 类
命名为 DA_TerrainData

4. 配置地形数据
双击打开 DA_TerrainData，配置：
Terrain Data 数组 - 添加7个元素：
地形建议颜色材质Ocean深蓝 (0.0, 0.1, 0.4)M_OceanCoast浅蓝 (0.2, 0.5, 0.8)M_CoastPlain浅绿 (0.4, 0.7, 0.3)M_PlainGrassland黄绿 (0.6, 0.7, 0.2)M_GrasslandDesert沙黄 (0.9, 0.8, 0.5)M_DesertTundra灰白 (0.7, 0.7, 0.6)M_TundraSnow白色 (0.95, 0.95, 1.0)M_Snow
Landform Data 数组 - 配置地貌模型：
地貌模型缩放HillsSM_Hill(1,1,1)MountainSM_Mountain(1,1,1.5)ForestSM_Tree(0.8,0.8,0.8)RainforestSM_TropicalTree(1,1,1)
Hex Settings：

HexBaseMesh: 你的六边形网格
HexSize: 100

5. 设置 GameMode

Edit → Project Settings → Maps & Modes
Default GameMode 设为 Civi_GameModeBase
或在 World Settings 中设置 GameMode Override

6. 配置 GameMode 默认值

基于 Civi_GameModeBase 创建蓝图子类 BP_CiviGameMode
在蓝图的 Details 面板中设置：

Terrain Data Asset: DA_TerrainData
Map Width: 74
Map Height: 46
调整噪声参数



7. 运行游戏
地图会在 BeginPlay 时自动生成和渲染。

素材资源推荐
免费资源

Kenney.nl - 低多边形风格素材包
OpenGameArt.org - 各种风格的游戏素材
Quaternius.com - 免费低多边形模型

付费资源

Unreal Marketplace 搜索 "hex tile" 或 "strategy game"
Synty Studios - 低多边形素材包
itch.io - 独立游戏素材

程序化生成
使用 UE 材质系统创建地形材质：

噪声节点生成纹理变化
顶点着色实现混合
视差贴图增加深度感


性能优化建议

使用实例化渲染 (已启用) - 相同类型的地块共享网格实例
LOD 系统 - 远处使用简化模型
遮挡剔除 - 不渲染被遮挡的地块
地块分块加载 - 只加载可见区域
