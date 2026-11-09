# ARCHITECTURE

DCC Packet Monitor Architecture Documentation

---

# 1. Overview

DCC Packet Monitor は
RP2040 (Raspberry Pi Pico) を使用した
NMRA DCC解析ツールである。

本プロジェクトは単なるパケット表示器ではなく、

- DCCデコーダ開発支援
- コマンドステーション解析
- Programming Track解析
- OPS Mode解析
- DCC信号品質評価
- DCC学習教材

として利用できることを目的としている。

設計では

- 可読性
- 保守性
- 教育的価値

を重視している。

---

# 2. Design Philosophy

本プロジェクトでは

「受信」
「デコード」
「パケット復元」
「解析」

を明確に分離する。

典型的なDCC解析ソフトでは
すべての処理を1つの巨大関数に集約しがちだが、

本プロジェクトでは
責務ごとのモジュール分割を採用している。

目的は

- デバッグ容易化
- 学習しやすさ
- 将来拡張の容易さ

である。

---

# 3. System Architecture

```text
DCC Input Signal
        │
        ▼
+------------------+
| dcc_rx           |
| Edge Detection   |
+------------------+
        │
        ▼
+------------------+
| dcc_decode       |
| Width -> Bit     |
+------------------+
        │
        ▼
+------------------+
| dcc_packet       |
| Bit -> Packet    |
+------------------+
        │
        ▼
+------------------+
| packet_analyzer  |
+------------------+
        │
 ┌──────┼────────────────────┐
 ▼      ▼          ▼         ▼
Loco  Accessory  Service   OPS
```

---

# 4. Data Flow

## Step 1

dcc_rx

GPIO入力を監視し、
信号エッジを取得する。

保持情報

```text
timestamp
pulse width
logic level
```

Bit判定は行わない。

---

## Step 2

dcc_decode

パルス幅から

```text
Bit 1
Bit 0
```

を判定する。

判定閾値は固定値ではなく
ユーザー設定可能。

理由

異なるコマンドステーションの
波形差異を評価するため。

---

## Step 3

dcc_packet

ビット列から
NMRA DCCパケットを復元する。

担当

```text
Preamble検出
Byte復元
Separator判定
Packet生成
```

CRC検査は行わない。

---

## Step 4

packet_analyzer

システム中心モジュール。

担当

```text
表示フィルタ
アドレスフィルタ
CRC確認
統計更新
解析モジュール振り分け
```

---

# 5. Module Responsibilities

## main.c

システムエントリポイント。

担当

- 初期化
- メインループ

のみ。

解析ロジックは持たない。

---

## menu.c

ユーザーインタフェース管理。

担当

- メニュー表示
- キー入力処理
- 設定変更
- 表示フィルタ設定
- アドレスフィルタ設定
- 統計画面管理

解析ロジックを持たない。

---

## monitor_config.c

設定保持モジュール。

保持項目

```text
表示フィルタ
アドレスフィルタ
RESET表示
IDLE表示
Bit閾値
プリアンブル数
```

---

## statistics.c

統計管理。

担当

```text
Packet Statistics
Signal Statistics
Histogram
Mode Analysis
```

オーバーフロー防止のため
飽和カウンタ方式を採用する。

---

# 6. Packet Analyzer Layer

パケット種別ごとに
専用モジュールを持つ。

理由

解析処理肥大化防止。

---

## loco_analyzer

機関車パケット解析。

対応

```text
Short Address
Long Address
128 Speed Step
Function Group 1 (F0-F4)
Function Group 2 (F5-F8)
Function Group 3 (F9-F12)
Function Expansion (F13-F68)
```

表示フィルタ
```text
Loco
Loco (Speed)
Loco (Function)
```


---

## accessory_analyzer

アクセサリデコーダ解析。

対応

```text
Basic Accessory Packet
アドレス復元
出力番号復元
Straight / Diverging 表示
```

NMRA Basic Accessory Packet の　Closed / Thrown　状態を
Straight / Diverging　の一般的なポイント表記も併記して表示する。

---

## service_analyzer

Programming Track解析。

対応

```text
Direct Byte Mode
Direct Bit Mode
Paged Mode
```

---

## ops_analyzer

Operations Mode Programming解析。

対応

```text
CV Write Packet
Short Address
Long Address
```

---

# 7. Statistics Design

## Packet Statistics

保持項目

```text
Total
Reset
Idle
Loco
Accessory
OPS
Service
CRC Error
```

本統計は
現在の表示条件
パケット種別フィルタ
アドレスフィルタ
を通過したパケットを対象とする。

目的

- パケット観測
- 流量解析
- プロトコル分析

---

## Signal Statistics

保持項目

```text
Bit0 Count
Bit1 Count
Histogram
Bit0 Mode
Bit1 Mode
```
本統計は「DCC 0/1信号 受信統計」で表示される。
パケット種別フィルタや アドレスフィルタの影響を受けず、受信した全DCC信号を対象に集計する。

目的

- コマンドステーション比較
- 信号品質評価
- DCC学習

---

# 8. Design Decisions

## なぜGPIO割り込みを使用しないのか

本プロジェクトは

「解析ツール」

であり、

最優先は

```text
理解しやすさ
移植しやすさ
保守しやすさ
```

である。

現状のポーリング方式でも
十分な性能を確保できるため、
シンプルな実装を優先した。

---

## なぜ処理を段階分離したのか

以下を独立検証可能にするため。

```text
受信
↓
Bit判定
↓
Packet復元
↓
Packet解析
```

問題発生時に
原因を局所化しやすい。

---

## なぜ統計を独立モジュール化したのか

解析処理へ統計コードを混在させると
責務が曖昧になるため。

また将来

```text
CSV出力
PC連携
GUI表示
```

などを追加しやすくする目的がある。

---

# 9. Future Extensions

候補

```text
Physical Register Mode

Extended Accessory Packet

RailCom解析

CSVログ保存

USB CDC高速転送

リアルタイムグラフ表示

信号品質スコア表示
```

---

# 10. Non Goals

本プロジェクトは

以下を目的としない。

```text
DCCコマンドステーション

DCCデコーダ

RailCom送信装置
```

あくまで

「DCC解析・学習・検証ツール」

である。

---

# 11. License

Copyright (C) 2026 Nucky

SPDX-License-Identifier: GPL-3.0-only