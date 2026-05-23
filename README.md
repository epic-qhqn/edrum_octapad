<div align="center">
  <h1>🥁 OctaPad E-Drum DIY</h1>
  <p><b>Feel the beat, build the rhythm.</b></p>
  <p><i>Cảm nhận nhịp điệu, tự tạo lối chơi.</i></p>
  
  ![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
  ![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)
  ![Hardware](https://img.shields.io/badge/Hardware-LGT8F328P-8A2BE2?style=for-the-badge)

  [English](#english) • [Tiếng Việt](#tiếng-việt)
</div>

---

<h2 id="english">🇬🇧 English</h2>

A fun, DIY 8-pad electronic drum kit project originally inspired by the awesome [DIY E Drum OctaPad video by Share 4u](https://youtu.be/tHEJwT9Mr2s). This repository contains a heavily optimized, rewritten firmware tailored specifically for the budget-friendly LGT8F328P (Type-C) board, though it still works perfectly on a standard Arduino Nano.

### ✨ Core Features
* **🥁** Reads 8 piezo sensors simultaneously without needing an external multiplexer.
* **🎛️** True-to-life volume response—hit soft for a tap, hit hard for a crash.
* **🛑** Built-in logic for a crash cymbal hand-choke and an active hi-hat foot pedal.
* **🔌** Spits out clean Serial MIDI data for apps like Hairless MIDI to route into your favorite DAW (FL Studio, Ableton, etc.).

### 💻 Under the Hood
* **Finite State Machine (FSM):** Dropped the clunky, blocking loops. Each pad runs on a strict 3-state system (`IDLE`, `CAPTURING`, `MUTED`) for zero-latency tracking.
* **Overflow Immunity:** Fixed the classic `micros()` 71-minute crash bug using standard unsigned integer math. You can jam for hours without the board freezing.
* **LGT8F328P Native:** Stripped out outdated AVR `ADCSRA` register hacks that cause chip crashes, relying instead on clean, native 10-bit ADC resolution.
* **Double-Trigger Kill:** Actively pulls pins to ground (`digitalWrite(LOW)`) during the mute phase to instantly kill residual piezo voltage and keep hits clean.

### 🚀 Getting Started
```bash
git clone [https://github.com/your-username/octapad-edrum.git](https://github.com/your-username/octapad-edrum.git)
```
1. Open the file in **Arduino IDE**.
2. Install the `LGT8fx Boards` package via Boards Manager.
3. Select **LGT8F328** (Clock: External 16 MHz), plug in your Type-C board, and hit **Upload**.

---

<h2 id="tiếng-việt">🇻🇳 Tiếng Việt</h2>

Dự án tự làm bộ trống điện tử 8 mặt (OctaPad) cực cuốn, được truyền cảm hứng từ video gốc của [Share 4u](https://youtu.be/tHEJwT9Mr2s). Repo này chứa bản code đã được "độ" lại cấu trúc, tối ưu hóa riêng cho mạch LGT8F328P (Type-C) giá rẻ nhưng vẫn tương thích hoàn hảo với Arduino Nano thông thường.

### ✨ Tính Năng Nổi Bật
* **🥁** Đọc mượt 8 cảm biến áp điện (Piezo) cùng lúc mà không cần đấu thêm IC ghép kênh lằng nhằng.
* **🎛️** Phản hồi âm thanh tự nhiên—gõ nhẹ kêu nhỏ, gõ mạnh kêu to.
* **🛑** Tích hợp tính năng dùng tay bóp mặt trống để ngắt âm (Crash Choke) và dậm pedal đổi tiếng Hi-Hat.
* **🔌** Xuất thẳng tín hiệu Serial MIDI, dễ dàng kết nối với Hairless MIDI để đánh trên các phần mềm như FL Studio, Ableton.

### 💻 Điểm Thú Vị Trong Code
* **Kiến Trúc Máy Trạng Thái (FSM):** Bỏ cách quét vòng lặp kẹt nhịp cũ, dùng 3 trạng thái (`IDLE`, `CAPTURING`, `MUTED`) để nhận tín hiệu gõ tức thì, không độ trễ.
* **Chống Tràn Bộ Nhớ:** Sửa triệt để lỗi sập nguồn sau 71 phút của hàm `micros()` bằng toán học. Đánh nguyên show 5 tiếng mạch vẫn không treo.
* **Chuẩn Form LGT8F328P:** Gỡ bỏ các lệnh ép xung thanh ghi AVR cũ dễ gây lỗi chip, sử dụng chuẩn phân giải ADC 10-bit an toàn và mượt mà.
* **Triệt Tiêu Nhiễu:** Ép chân tín hiệu xuống đất (GND) ngay sau mỗi cú hit để xả điện dư, chấm dứt tình trạng gõ 1 cái kêu 2 tiếng (double-trigger).

### 🚀 Cài Đặt
```bash
git clone [https://github.com/your-username/octapad-edrum.git](https://github.com/your-username/octapad-edrum.git)
```
1. Mở file code bằng phần mềm **Arduino IDE**.
2. Cài đặt gói `LGT8fx Boards` trong mục Boards Manager.
3. Chọn board **LGT8F328** (Clock: External 16 MHz), cắm cáp Type-C và nhấn **Upload**.
