import { importRuntimeModule } from "/Users/maoxiaoxi/.codex/plugins/cache/openai-primary-runtime/presentations/26.826.12353/skills/presentations/container_tools/runtime_helpers.mjs";
const { FileBlob, PresentationFile } = await importRuntimeModule("@oai/artifact-tool");

const presentation = await PresentationFile.importPptx(
  await FileBlob.load("/Users/maoxiaoxi/Documents/code/C++/Qt/RealSense_Snap/tmp/presentation-business-review/template-starter.pptx"),
);

const snapshot = await presentation.inspect({
  kind: "textbox,shape,table,chart",
  include: "id,slide,name,textPreview,placeholder,bbox,rows,cols,chartType",
  maxChars: 12000,
});
console.log(snapshot.ndjson.split(/\n/).slice(0, 20).join("\n"));
const ids = [];
for (const line of snapshot.ndjson.split(/\n/)) {
  if (!line.trim()) continue;
  const record = JSON.parse(line);
  if (record.id && ids.length < 4) ids.push(record.id);
}
for (const id of ids) {
  try {
    const obj = presentation.resolve(id);
    const proto = Object.getPrototypeOf(obj);
    console.log(id, obj?.constructor?.name, Object.getOwnPropertyNames(proto).filter((x) => !x.startsWith("_")).slice(0, 80).join(","));
    if (obj.text) {
      const textProto = Object.getPrototypeOf(obj.text);
      console.log(id, "text", Object.getOwnPropertyNames(textProto).filter((x) => !x.startsWith("_")).slice(0, 80).join(","));
    }
  } catch (error) {
    console.log(id, "ERROR", error.message);
  }
}
