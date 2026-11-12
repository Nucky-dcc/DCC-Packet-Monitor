# 変更履歴

このファイルでは DCC Monitor の主な変更内容を記録します。

---

### v1.2.0

プリアンブル数の初期値を14から10に変更

---

### v1.1.0

NMRA Baseline Speed Instruction (14Step / 28Step) の解析機能を強化し、
実機検証に基づく速度表示の改善を実施

#### 追加

- 14 Speed Step解析対応
- 28 Speed Step解析対応
- Advanced Consist Packet検出対応
- NMRA S-9.2速度テーブル対応
- F0 ON時STOP(E-STOP)判定対応
- F0 ON時STOP(I)・E-STOP(I)判定対応

#### 改善

- 機関車速度命令表示を改善
- 14Step解釈と28Step解釈の同時表示を実装
- 28Step速度表示の精度向上
- 最大速度表示をNMRA準拠へ修正
- 速度命令表示フォーマットを簡潔化

#### 検証

- Digitrax DCS50Kによる14Step動作確認
- Digitrax DCS50Kによる28Step動作確認
- STOP / E-STOP動作確認
- F0 ON/OFF状態での速度解析確認

---

## v1.0.0

初回公開版

### 基本機能

- Raspberry Pi Pico を使用した DCC信号受信
- DCCビット復元
- DCCパケット復元
- パケットモニタ表示
- パケット通番表示
- パケット間隔表示

### パケット解析

- CRCチェック
- Reset Packet解析
- Idle Packet解析
- Loco Packet解析
- Accessory Packet解析
- Service Mode Packet解析
- OPS Mode Packet解析

### メニューシステム

- 階層型メニュー
- ESCによる画面遷移
- パケット監視画面
- パケット統計画面
- 信号統計画面
- 設定画面

### フィルタ機能

- パケット種別フィルタ
  - ALL
  - LOCO
  - LOCO(Speed)
  - LOCO(Function)
  - ACCESSORY
  - OPS

- アドレスフィルタ

### 統計機能

- 総受信パケット数
- Reset Packet数
- Idle Packet数
- Loco Packet数
- Accessory Packet数
- Service Packet数
- OPS Packet数
- CRC Error数

### 信号解析機能

- Bit0カウント
- Bit1カウント
- Bit0パルス幅統計
- Bit1パルス幅統計
- パルス幅ヒストグラム
- Bit0 Mode値算出
- Bit1 Mode値算出

### 設定機能

- Bit判定閾値変更
- プリアンブル判定数変更
- Reset Packet表示ON/OFF
- Idle Packet表示ON/OFF

