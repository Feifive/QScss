import { mkdir, readdir, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";
import * as sass from "sass";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const themeDir = path.join(root, "styles", "themes");
const outputDir = path.join(root, "dist", "qss");
const clean = process.argv.includes("--clean");

if (clean) {
  await rm(outputDir, { recursive: true, force: true });
}

await mkdir(outputDir, { recursive: true });

const entries = (await readdir(themeDir, { withFileTypes: true }))
  .filter((entry) => entry.isFile())
  .map((entry) => entry.name)
  .filter((name) => name.endsWith(".scss") && !name.startsWith("_"));

if (entries.length === 0) {
  throw new Error(`No SCSS theme entries found in ${themeDir}`);
}

for (const entry of entries) {
  const source = path.join(themeDir, entry);
  const target = path.join(outputDir, entry.replace(/\.scss$/, ".qss"));

  const result = sass.compile(source, {
    loadPaths: [path.join(root, "styles")],
    style: "expanded",
    charset: false,
    sourceMap: false,
  });

  await writeFile(target, result.css);
  console.log(`${path.relative(root, source)} -> ${path.relative(root, target)}`);
}

const manifest = entries.map((entry) => {
  const id = entry.replace(/\.scss$/, "");
  return {
    id,
    name: toDisplayName(id),
    file: `${id}.qss`,
  };
});

await writeFile(
  path.join(outputDir, "themes.json"),
  `${JSON.stringify(manifest, null, 2)}\n`
);
console.log(`generated ${path.relative(root, path.join(outputDir, "themes.json"))}`);

function toDisplayName(id) {
  return id
    .split(/[-_]/)
    .filter(Boolean)
    .map((part) => `${part.charAt(0).toUpperCase()}${part.slice(1)}`)
    .join(" ");
}
