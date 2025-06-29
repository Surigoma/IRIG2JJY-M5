[English README is here](README.md)

# IRIG2JJY-M5

M5Stack (M5StickC Plus) を用いて IRIG 時刻信号のデコードと JJY 信号の生成を行うプロジェクトです。

## 特徴

- IRIG 信号（IRIG-B）のデコード
- JJY 信号の生成
- リアルタイムクロック同期
- LCD によるデバッグ表示
- マルチタスク・割り込み安全設計

## ハードウェア要件

- M5Stack M5StickC Plus（または互換品）
- IRIG 信号入力（GPIO36）
- JJY 出力（GPIO26）
- LED 出力（GPIO10）

## はじめかた

1. **本リポジトリをクローン**
2. **PlatformIO または Arduino IDE で開く**
3. **ボード設定を M5StickC Plus に合わせる**
4. **ビルドして書き込み**

## ファイル構成

- `src/` : メインソースコード
- `include/` : ヘッダファイル
- `lib/` : 外部ライブラリ（必要に応じて）
- `test/` : テストコード

## 主な構成要素

- `main.cpp` : エントリポイント、マルチタスク設定、メインループ
- `IRIG.hpp/cpp` : IRIG 信号デコーダ
- `JJY.hpp/cpp` : JJY 信号ジェネレータ
- `clockManager.hpp/cpp` : クロック・タイミング管理
- `define.hpp` : ピン定義

## 使い方

- IRIG 入力をデコードし、システムクロックを同期します。
- 同期した時刻に基づき JJY 出力を生成します。
- デバッグ情報は LCD に表示されます。
- ボタン A を押すとデバイスがリセットされます。

## ライセンス

MIT License (c) 2025 Surigoma

[English README is here](README.md)
