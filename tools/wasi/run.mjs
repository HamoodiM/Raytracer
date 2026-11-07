// Runs a wasm32-wasi program under Node's built-in WASI.
//   node tools/wasi/run.mjs build-wasm/raytracer_cli.wasm --scene scenes/x.json --out out.png
// The current directory is preopened, so relative paths (scenes, textures,
// output images) resolve against it. Run from the repository root.
import { readFile } from "node:fs/promises";
import { WASI } from "node:wasi";

const [file, ...args] = process.argv.slice(2);
if (!file) {
  console.error("usage: node tools/wasi/run.mjs program.wasm [args...]");
  process.exit(2);
}
const wasi = new WASI({
  version: "preview1",
  args: [file, ...args],
  env: {},
  preopens: { ".": process.cwd() },
  returnOnExit: true,
});
const module = await WebAssembly.compile(await readFile(file));
const instance = await WebAssembly.instantiate(module, wasi.getImportObject());
process.exitCode = wasi.start(instance);
