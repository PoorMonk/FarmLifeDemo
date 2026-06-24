# FarmLifeDemo
## 2026.5.27 - 2026.5.28
- 创建 UE 5.5 C++ 第三人称项目。
- 搭建一个极小灰盒农场场景。
- 放置玩家、农田地块、床、出售箱占位物。
- 实现最小可用的交互系统。
- 玩家可以靠近对象，看到交互提示，并按键触发对象响应。

## 2026.6.15
- 实现农田地块状态机:Raw → Tilled → Seeded → Mature → Tilled。
- 引入工具切换:锄头 / 水壶 / 种子 / 空手,通过 1/2/3/] 切换。
- `IToolHolder` 接口解耦地块和角色,为后续接入背包预留接口。
- `UTimeSubsystem` 时间桩 + Debug 跨天键 `]`,验证浇水推进生长规则。
- DMI 颜色实时表达地块状态。
## 2026.6.24
- 引入 `UCropDataAsset`(继承 `UPrimaryDataAsset`),作物的生长天数和阶段 Mesh 从地块字段迁到数据资产。
- 地块新增 `CropMesh` 子组件,从作物配置按 `GrowthProgress / GrowthDaysRequired` 比例切阶段 Mesh。
- `IToolHolder` 扩展 `GetCurrentSeed`,Character 加 `CurrentSeed` 占位字段;Step 5 接背包后切到 `UInventoryComponent`,地块侧零改动。
- 土地视觉职责简化:`PlotMesh` 只表达 Raw / `bWatered`,作物表达全交给 `CropMesh`。
- 新增作物只需在 `Content/Data/Crops/` 下复制资产并改字段,零 C++ 改动。
