import fs from "node:fs/promises";
import path from "node:path";
import { importRuntimeModule } from "/Users/maoxiaoxi/.codex/plugins/cache/openai-primary-runtime/presentations/26.826.12353/skills/presentations/container_tools/runtime_helpers.mjs";

const { FileBlob, PresentationFile } = await importRuntimeModule("@oai/artifact-tool");

const TMP_DIR = "/Users/maoxiaoxi/Documents/code/C++/Qt/RealSense_Snap/tmp/presentation-business-review";
const STARTER = path.join(TMP_DIR, "template-starter.pptx");
const FINAL = "/Users/maoxiaoxi/Documents/code/C++/Qt/RealSense_Snap/RealSense_Snap_实习生一个月学习成果展示汇报.pptx";
const RENDER_DIR = path.join(TMP_DIR, "final-render");
const LAYOUT_DIR = path.join(TMP_DIR, "final-layout");

async function writeBlob(filePath, blob) {
  await fs.mkdir(path.dirname(filePath), { recursive: true });
  await fs.writeFile(filePath, Buffer.from(await blob.arrayBuffer()));
}

function parseInspect(ndjson) {
  return ndjson
    .split(/\r?\n/)
    .filter((line) => line.trim())
    .map((line) => JSON.parse(line))
    .filter((record) => record.kind !== "notice");
}

function recordsBySlide(records, slide, kind) {
  return records.filter((record) => record.slide === slide && record.kind === kind);
}

function textboxes(records, slide) {
  return recordsBySlide(records, slide, "textbox");
}

function setText(presentation, records, slide, index, text) {
  const record = textboxes(records, slide)[index];
  if (!record) throw new Error(`Missing textbox slide ${slide} index ${index}`);
  const shape = presentation.resolve(record.id);
  shape.text.set(text);
}

function setAllText(presentation, records, slide, values) {
  values.forEach((value, index) => setText(presentation, records, slide, index, value));
}

function deleteFooters(presentation, records) {
  for (const record of records) {
    if ((record.kind === "shape" || record.kind === "textbox") && record.placeholder === "footer") {
      try {
        presentation.resolve(record.id).delete();
      } catch {
        // Some imported placeholder stubs may already be inert after export.
      }
    }
  }
}

function setTable(presentation, records, slide, values) {
  const tableRecord = recordsBySlide(records, slide, "table")[0];
  if (!tableRecord) return;
  const table = presentation.resolve(tableRecord.id);
  for (let r = 0; r < values.length; r += 1) {
    for (let c = 0; c < values[r].length; c += 1) {
      table.cells.set(r, c, values[r][c]);
    }
  }
}

function setChart(presentation, records, slide, categories, seriesA, seriesB) {
  const chartRecord = recordsBySlide(records, slide, "chart")[0];
  if (!chartRecord) return;
  const chart = presentation.resolve(chartRecord.id);
  try {
    chart.categories = categories;
    chart.series.getItemAt(0).name = seriesA.name;
    chart.series.getItemAt(0).values = seriesA.values;
    if (seriesB && chart.series.getItemAt(1)) {
      chart.series.getItemAt(1).name = seriesB.name;
      chart.series.getItemAt(1).values = seriesB.values;
    }
    chart.yAxis = { min: 0, max: 5, majorUnit: 1, numberFormatCode: "0" };
    chart.hasLegend = true;
  } catch {
    // Imported combo charts may expose only partial editability; the adjacent
    // cards still carry the audience-facing conclusion.
  }
}

function setNotes(presentation, slideIndex, notes) {
  const slide = presentation.slides.getItem(slideIndex - 1);
  slide.speakerNotes.textFrame.setText(notes);
  slide.speakerNotes.setVisible(true);
}

const presentation = await PresentationFile.importPptx(await FileBlob.load(STARTER));
const inspect = await presentation.inspect({
  kind: "textbox,shape,table,chart,notes",
  include: "id,slide,name,textPreview,text,placeholder,bbox,rows,cols,chartType",
  maxChars: 120000,
});
const records = parseInspect(inspect.ndjson);

deleteFooters(presentation, records);

setAllText(presentation, records, 1, [
  "RealSense Snap\n实习生一个月学习成果展示",
  "一个月内围绕 Qt Quick、Intel RealSense、OpenCV、ONNX Runtime 与 YOLO 模型，完成从设备采集、实时处理、AI 分割、背景替换到拍照保存的系统化学习与实践。",
  "2026.08",
  "公司实习月度汇报"
]);

setAllText(presentation, records, 2, [
  "2",
  "本次汇报沿着“目标、流程、技术、反思”展开",
  "汇报导航"
]);
setTable(presentation, records, 2, [
  ["01", "项目背景与目标", "为什么做这个系统"],
  ["02", "整体架构与线程模型", "前端、控制桥、Worker 如何协作"],
  ["03", "逐帧处理主流程", "每帧图像怎样完成采集到显示"],
  ["04", "深度处理与 AI 分割", "传统视觉和模型推理如何互补"],
  ["05", "交互、扩展与工程亮点", "怎样把能力包装成应用"],
  ["06", "问题复盘与下一步", "学习收获和后续优化"]
]);

setAllText(presentation, records, 3, [
  "我把学习结果落在一个可运行的实时深度拍照系统上：理解 Qt 事件驱动和线程通信，接入 RealSense RGB/Depth 数据流，完成 OpenCV 深度滤波、YOLO 人像分割、背景替换、深度伪彩叠加和照片保存。",
  "3",
  "完成度",
  "核心闭环已经打通，系统能从设备输入连续产生可见预览，并响应摄像头刷新、设备选择、背景切换、透明度调节和拍照保存等交互。",
  "关键结论",
  "一个月学习成果不是单点算法 demo，而是把 UI、设备、图像处理、模型推理和工程异常处理串成了完整应用。",
  "一个月成果总览",
  "Executive Summary",
  "已形成系统认知",
  "后续最需要强化的是相机状态机、模块边界和测试体系，降低设备异常与模型推理带来的不确定性。",
  "下一步判断",
  "围绕 RealSense 输入、视觉处理、AI 推理、图像合成、UI 交互和照片输出，说明我学到了什么、做通了什么、还在思考什么。"
]);

setAllText(presentation, records, 4, [
  "4",
  "项目目标是把深度相机能力转化为可交互的拍照体验",
  "RealSense 同时提供彩色图和深度图，项目目标是在桌面端建立采集、处理、预览和保存的闭环。",
  "普通 RGB 摄像头难以稳定区分前景和背景；深度数据与 AI 分割结合后，可以提升复杂背景下的人像提取稳定性。",
  "设备输入",
  "枚举并选择 RealSense 设备，采集 color 和 depth 两路数据。",
  "图像处理",
  "对深度帧滤波、阈值、形态学处理，提取有效前景范围。",
  "AI 补强",
  "使用 YOLO segmentation 生成人像 mask，修正深度阈值不足。",
  "用户输出",
  "完成背景替换、深度叠加、实时预览和照片保存。"
]);

setAllText(presentation, records, 5, [
  "5",
  "系统采用 Qt 前端、控制桥、后台 Worker 和视觉工具类分层协作",
  "Main.qml 与 VideoItem 承载实时画面、设备选择、滑块、背景选择和拍照按钮。",
  "这种分层让我理解到：可运行应用不仅要写算法，还要安排对象生命周期、线程边界、资源路径和错误反馈。",
  "承载实时画面、设备选择、滑块、背景选择和拍照按钮。",
  "UI 层",
  "CameraController 转发 QML 请求，CameraWorker 负责设备、帧处理和状态通知。",
  "调度层",
  "FilterProcessing、yoloSeg、facialLandmark 与 ImageBackgroundProvider 分别封装视觉能力。",
  "算法层"
]);

setAllText(presentation, records, 6, [
  "6",
  "控制桥\n转发 UI 请求并维护状态。",
  "Worker 线程\n后台启动 pipeline、处理帧并返回结果。",
  "线程模型保证了界面响应和相机处理互不阻塞",
  "QML 渲染、用户操作和 RealSense 帧循环被分开；处理完成后用 frameReady(QImage) 通知 VideoItem 更新。",
  "UI 线程\nQML 渲染界面并接收交互。",
  "Qt::QueuedConnection 让跨线程调用进入事件队列，是这页最关键的工程实践。"
]);

setAllText(presentation, records, 7, [
  "输入链路：QTimer 触发 processFrame；RealSense pipeline 获取 frameset；depth 对齐到 color；读取 depth frame 和 color frame；将 color frame 转成 OpenCV BGR。",
  "7",
  "每一帧都会经过采集、对齐、滤波、分割、合成和显示",
  "处理链路：RealSense SDK 先做 disparity、spatial、temporal 等滤波；OpenCV 再做平滑、阈值和形态学；隔帧执行 YOLO 人像分割，生成背景 mask，清理背景深度。",
  "核心程序流程：从设备帧到用户可见预览，关键是保证每一步输入输出类型、尺寸和线程边界清晰。"
]);

setAllText(presentation, records, 8, [
  "8",
  "深度图处理的价值，是把原始距离数据变成稳定的前景判断依据",
  "RealSense 原始深度存在噪声、空洞和边缘抖动。项目先用 SDK 滤波器稳定深度，再用 OpenCV 清理无效值、限定深度范围、闭运算和腐蚀，最终只保留可用于合成的前景深度。"
]);

setAllText(presentation, records, 9, [
  "9",
  "YOLO 分割补强复杂背景下的前景判断",
  "RGB 图像推理出 person mask，再与深度图结合，清理背景深度。",
  "关键学习点：模型调用要拆成预处理、推理、后处理，并处理尺寸还原、NMS 和 mask 合成。",
]);
setTable(presentation, records, 9, [
  ["环节", "输入", "核心处理", "输出"],
  ["预处理", "OpenCV BGR", "letterbox、归一化、CHW", "ONNX tensor"],
  ["推理", "seg ONNX", "Session Run", "检测与原型"],
  ["筛选", "预测结果", "置信度、person、NMS", "候选区域"],
  ["合成", "mask 系数", "sigmoid、resize", "person mask"],
  ["约束", "person mask", "框内保留、形态学", "稳定 mask"],
  ["融合", "mask + depth", "取反清理背景深度", "合成深度"],
  ["策略", "连续帧", "隔帧推理", "实时性更稳"],
  ["模型", "resources/models", "yolo26n-seg.onnx", "人像分割能力"],
  ["框架", "C++", "ONNX Runtime / OpenCV", "工程封装"]
]);

setAllText(presentation, records, 10, [
  "10",
  "背景替换和深度叠加把算法结果变成用户能理解的预览",
  "背景图按当前画面尺寸缓存，避免重复 resize。",
  "缓存背景",
  "foregroundMask 来自有效深度范围，背景区域用图片替换。",
  "前景判断",
  "distanceTransform 计算边缘距离，平滑 alpha 减少硬边。",
  "边缘羽化",
]);
setChart(presentation, records, 10, ["采集", "滤波", "分割", "合成"], { name: "实现深度", values: [3, 4, 4, 5] }, { name: "学习难度", values: [3, 4, 5, 4] });

setAllText(presentation, records, 11, [
  "11",
  "人脸关键点扩展让我看到模型能力如何继续叠加到系统中",
  "5点",
  "左眼、右眼、鼻尖、左嘴角、右嘴角",
  "模型",
  "yolov8n-face.onnx 由 OpenCV DNN 加载推理",
  "facialLandmark 工具类封装人脸框、置信度和 5 个关键点；后处理包含 letterbox、DFL 解码、sigmoid 置信度、NMS 和坐标映射。这为后续人脸增强、美颜或姿态判断留下扩展入口。"
]);
setChart(presentation, records, 11, ["UI", "设备", "深度", "AI"], { name: "掌握度", values: [4, 3, 4, 4] }, { name: "待提升", values: [2, 3, 3, 3] });

setAllText(presentation, records, 12, [
  "熟悉环境\n熟悉 Qt Quick、CMake、RealSense 依赖和项目目录，先把应用启动、设备枚举和界面显示链路跑通。",
  "12",
  "项目学习路径从“跑通设备”逐步走向“理解系统边界”",
  "第 2-3 周\n深入 CameraWorker 与 processFrame，理解 RGB/Depth 对齐、OpenCV Mat 转换、深度滤波、YOLO 分割和背景合成。",
  "第 4 周\n补充人脸关键点扩展、错误检查、资源路径和 demo 验证，开始从模块职责、实时性和稳定性角度复盘。",
  "第 1 周",
  "第 2-3 周",
  "第 4 周"
]);

setAllText(presentation, records, 13, [
  "13",
  "当前项目可运行，但相机状态、模块边界和测试仍值得继续优化",
  "拆分 Worker 职责",
  "完善 start/stop 状态机",
  "增加模型加载错误提示",
  "显式指定 640x480 30fps",
  "补充 demo 与单元测试",
  "整理工程文档"
]);
setTable(presentation, records, 13, [
  ["已解决/已识别", "相机稳定性", "架构边界", "用户反馈", "测试验证"]
]);

setAllText(presentation, records, 14, [
  "14",
  "一个月的最大收获，是把“会调用库”推进到“能解释完整系统流程”",
  "RealSense Snap 让我把 Qt Quick、C++ Worker、RealSense SDK、OpenCV 和 YOLO 模型放进同一条实时链路中理解。下一阶段会围绕稳定性、模块化和可测试性继续推进。"
]);

const sourceNotes = [
  "[Sources]",
  "docs/项目汇报PPT制作大纲.md",
  "README.md",
  "CMakeLists.txt",
  "src/main.cpp",
  "src/core/CameraWorker.cpp",
  "src/core/cameracontroller.cpp",
  "src/core/videoitem.cpp",
  "src/processing/filterprocessing.cpp",
  "src/utils/yoloseg.cpp",
  "src/utils/faciallandmark.cpp",
  "src/background/imagebackgroundprovider.cpp",
  "qml/Main.qml",
].join("\n");
for (let i = 1; i <= 14; i += 1) {
  setNotes(presentation, i, sourceNotes);
}

await fs.mkdir(RENDER_DIR, { recursive: true });
await fs.mkdir(LAYOUT_DIR, { recursive: true });
for (const [index, slide] of presentation.slides.items.entries()) {
  const stem = `slide-${String(index + 1).padStart(2, "0")}`;
  await writeBlob(path.join(RENDER_DIR, `${stem}.png`), await presentation.export({ slide, format: "png", scale: 1 }));
  await fs.writeFile(path.join(LAYOUT_DIR, `${stem}.layout.json`), await (await slide.export({ format: "layout" })).text());
}

const montage = await presentation.export({ format: "webp", montage: true, scale: 1 });
await writeBlob(path.join(TMP_DIR, "final-montage.webp"), montage);

const pptx = await PresentationFile.exportPptx(presentation);
await pptx.save(FINAL);
console.log(FINAL);
