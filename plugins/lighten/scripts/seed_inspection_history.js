/**
 * 向 lighten_inspection.db 批量插入模拟生产检测历史数据。
 *
 * 用法:
 *   node plugins/lighten/scripts/seed_inspection_history.js [db路径] [条数]
 *
 * 默认: build/bin/Debug/data/lighten_inspection.db, 1000 条
 */

'use strict';

const path = require('path');
const { DatabaseSync } = require('node:sqlite');

const DEFAULT_DB = path.resolve(
  __dirname,
  '../../../build/bin/Debug/data/lighten_inspection.db',
);
const args = process.argv.slice(2);
let DB_PATH = DEFAULT_DB;
let COUNT = 1000;
if (args.length >= 1 && args[0].toLowerCase().endsWith('.db')) {
  DB_PATH = path.resolve(args[0]);
  if (args.length >= 2) COUNT = parseInt(args[1], 10) || 1000;
} else if (args.length >= 1) {
  COUNT = parseInt(args[0], 10) || 1000;
}

const MODELS = ['A01', 'A02', 'B01', 'B03', 'C05', 'D07'];
const SPECIAL_DETECT = ['非边缘缺陷', '边缘异常', '自定义项-A'];

function randInt(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

function pick(arr) {
  return arr[randInt(0, arr.length - 1)];
}

function weightedPick(items) {
  const total = items.reduce((s, [, w]) => s + w, 0);
  let r = Math.random() * total;
  for (const [value, weight] of items) {
    r -= weight;
    if (r <= 0) return value;
  }
  return items[items.length - 1][0];
}

function pad(n, width) {
  return String(n).padStart(width, '0');
}

function formatDateTime(d) {
  const y = d.getFullYear();
  const m = pad(d.getMonth() + 1, 2);
  const day = pad(d.getDate(), 2);
  const h = pad(d.getHours(), 2);
  const min = pad(d.getMinutes(), 2);
  const s = pad(d.getSeconds(), 2);
  return `${y}-${m}-${day} ${h}:${min}:${s}`;
}

function randomInspectedAt(baseDate, dayOffset) {
  const d = new Date(baseDate);
  d.setDate(d.getDate() - dayOffset);
  d.setHours(randInt(8, 21), randInt(0, 59), randInt(0, 59), 0);
  return formatDateTime(d);
}

function genDefectCounts(hasDefects) {
  const fields = [
    'dead_led_r', 'dead_led_g', 'dead_led_b',
    'dim_led_r', 'dim_led_g', 'dim_led_b',
    'bright_led_r', 'bright_led_g', 'bright_led_b',
    'color_var_r', 'color_var_g', 'color_var_b',
    'series_led_r', 'series_led_g', 'series_led_b',
    'mixed_color_r', 'mixed_color_g', 'mixed_color_b',
  ];
  const values = {};
  for (const f of fields) values[f] = 0;

  let black_screen_light_count = 0;
  let defect_type = 0;

  if (!hasDefects) {
    return { ...values, black_screen_light_count, defect_type, defect_total_count: 0 };
  }

  const categories = [
    { bit: 1 << 0, keys: ['dead_led_r', 'dead_led_g', 'dead_led_b'] },
    { bit: 1 << 1, keys: ['dim_led_r', 'dim_led_g', 'dim_led_b'] },
    { bit: 1 << 2, keys: ['bright_led_r', 'bright_led_g', 'bright_led_b'] },
    { bit: 1 << 3, keys: ['color_var_r', 'color_var_g', 'color_var_b'] },
    { bit: 1 << 4, keys: ['series_led_r', 'series_led_g', 'series_led_b'] },
    { bit: 1 << 5, keys: ['mixed_color_r', 'mixed_color_g', 'mixed_color_b'] },
    { bit: 1 << 6, keys: [] },
    { bit: 1 << 7, keys: [] },
  ];

  const activeCount = randInt(1, 3);
  const shuffled = categories.sort(() => Math.random() - 0.5).slice(0, activeCount);

  for (const cat of shuffled) {
    defect_type |= cat.bit;
    if (cat.bit === 1 << 6) {
      black_screen_light_count = randInt(1, 8);
      continue;
    }
    if (cat.bit === 1 << 7) continue;
    for (const key of cat.keys) {
      if (Math.random() < 0.55) values[key] = randInt(1, 25);
    }
  }

  if (Math.random() < 0.08) defect_type |= 1 << 7;

  let defect_total_count = black_screen_light_count;
  for (const f of fields) defect_total_count += values[f];

  if (defect_total_count === 0 && !(defect_type & (1 << 7))) {
    values[pick(fields)] = randInt(1, 5);
    defect_type |= 1 << 0;
    defect_total_count = values.dead_led_r + values.dead_led_g + values.dead_led_b
      + values.dim_led_r + values.dim_led_g + values.dim_led_b
      + values.bright_led_r + values.bright_led_g + values.bright_led_b
      + values.color_var_r + values.color_var_g + values.color_var_b
      + values.series_led_r + values.series_led_g + values.series_led_b
      + values.mixed_color_r + values.mixed_color_g + values.mixed_color_b
      + black_screen_light_count;
  }

  return { ...values, black_screen_light_count, defect_type, defect_total_count };
}

function genFlashResult() {
  return weightedPick([
    [0, 0.70],
    [1, 0.30],
  ]);
}

function genRow(i, baseDate) {
  const model = pick(MODELS);
  const dayOffset = randInt(0, 59);
  const inspected_at = randomInspectedAt(baseDate, dayOffset);
  const datePart = inspected_at.slice(0, 10).replace(/-/g, '');
  const board_sn = `${model}-${datePart}-${pad(i + 1, 5)}`;

  const inspection_result = weightedPick([
    [0, 0.72],
    [1, 0.23],
    [2, 0.05],
  ]);

  let inspect_times = 1;
  let rw_count = 0;
  let hasDefects = inspection_result !== 0;

  if (inspection_result === 2) {
    rw_count = randInt(1, 3);
    inspect_times = rw_count + 1;
    hasDefects = Math.random() < 0.6;
  } else if (inspection_result === 1) {
    inspect_times = Math.random() < 0.2 ? 2 : 1;
  }

  const defects = genDefectCounts(hasDefects);
  const flash_result = genFlashResult();
  let special_detect = null;
  if (defects.defect_type & (1 << 7)) {
    special_detect = pick(SPECIAL_DETECT);
  }

  const inspect_duration_ms = randInt(8500, 95000);

  return {
    product_model: model,
    board_sn,
    inspection_result,
    inspect_times,
    rw_count,
    inspect_duration_ms,
    ...defects,
    flash_result,
    special_detect,
    inspected_at,
  };
}

const INSERT_SQL = `
INSERT INTO lighten_inspection_history (
  product_model, board_sn, inspection_result, inspect_times, rw_count, inspect_duration_ms,
  dead_led_r, dead_led_g, dead_led_b,
  dim_led_r, dim_led_g, dim_led_b,
  bright_led_r, bright_led_g, bright_led_b,
  color_var_r, color_var_g, color_var_b,
  series_led_r, series_led_g, series_led_b,
  mixed_color_r, mixed_color_g, mixed_color_b,
  black_screen_light_count, defect_total_count, defect_type,
  flash_result, special_detect, inspected_at
) VALUES (
  @product_model, @board_sn, @inspection_result, @inspect_times, @rw_count, @inspect_duration_ms,
  @dead_led_r, @dead_led_g, @dead_led_b,
  @dim_led_r, @dim_led_g, @dim_led_b,
  @bright_led_r, @bright_led_g, @bright_led_b,
  @color_var_r, @color_var_g, @color_var_b,
  @series_led_r, @series_led_g, @series_led_b,
  @mixed_color_r, @mixed_color_g, @mixed_color_b,
  @black_screen_light_count, @defect_total_count, @defect_type,
  @flash_result, @special_detect, @inspected_at
)`;

function main() {
  const db = new DatabaseSync(DB_PATH);
  db.exec('PRAGMA foreign_keys = ON');

  const before = db.prepare('SELECT COUNT(*) AS c FROM lighten_inspection_history').get().c;
  const insert = db.prepare(INSERT_SQL);
  const baseDate = new Date();

  db.exec('BEGIN');
  try {
    for (let i = 0; i < COUNT; ++i) insert.run(genRow(i, baseDate));
    db.exec('COMMIT');
  } catch (err) {
    db.exec('ROLLBACK');
    throw err;
  }

  const after = db.prepare('SELECT COUNT(*) AS c FROM lighten_inspection_history').get().c;
  const stats = db.prepare(`
    SELECT inspection_result, COUNT(*) AS c
    FROM lighten_inspection_history
    GROUP BY inspection_result
    ORDER BY inspection_result
  `).all();

  console.log(`DB: ${DB_PATH}`);
  console.log(`Inserted: ${COUNT} rows (${before} -> ${after})`);
  console.log('Result distribution:', stats);
  db.close();
}

main();
