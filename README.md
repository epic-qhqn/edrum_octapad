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

An 8-pad electronic drum kit firmware based on the DIY E Drum OctaPad video by [Share 4u](https://youtu.be/tHEJwT9Mr2s). This repository provides an optimized rewrite for the LGT8F328P (Type-C) microcontroller, while maintaining full compatibility with the standard Arduino Nano.

### 🧰 Hardware Components
| Component | Quantity | Notes |
| :--- | :---: | :--- |
| LGT8F328P (Type-C) / Arduino Nano | 1 | Main microcontroller |
| Piezoelectric Sensors (27mm/35mm) | 8 | Strike detection |
| 1MΩ Resistors | 8 | Piezo discharge |
| 5.1V Zener diodes | 8 | Voltage clipping / IC protection |
| Perfboard & Jumper Wires | 1 set | For ground bus and signal routing |
| 1.0cm PVC Foam / Rubber Pads | 8 | Strike pads (Dampening material) |

### ✨ Core Features
* Reads signals from 8 piezo sensors concurrently without requiring external multiplexer hardware.
* Incorporates a material thickness compensation algorithm that adjusts the ADC ceiling. This ensures that thick pads (e.g., 1.0cm) can still output the full 30-127 MIDI velocity range without excessive striking force.
* Supports physical hardware inputs for a Crash Cymbal hand-choke (Note-Off) and a Hi-Hat foot pedal to toggle between Open and Closed states.
* Outputs Serial data mapped to the General MIDI (GM) standard, ensuring plug-and-play compatibility with DAWs and VSTs like FL Studio's FPC or EZDrummer via Hairless MIDI.

### 💻 Technical Details
* Signal processing is managed by a strict 3-state Finite State Machine (`IDLE`, `CAPTURING`, `MUTED`) rather than blocking delay loops. This brings the peak capture window down to 500µs, minimizing input latency.
* Sympathetic vibrations (crosstalk) between adjacent pads are filtered mathematically using an algorithm based on physical sensor distance and peak timing.
* The standard `micros()` overflow bug, which typically causes a system halt after 71 minutes, is resolved via unsigned integer arithmetic for stable, long-term operation.
* Analog pins are actively pulled to ground (`digitalWrite(LOW)`) during the mute phase to quickly discharge residual piezo capacitance and prevent double-triggering.

### 🚀 Setup & Calibration
1. Open the file in **Arduino IDE**.
2. Configure your physical parameters in the code:
   * **Boost/Thickness:** Adjust `MATERIAL_THICKNESS_CM` (e.g., `1.0`). The system uses this to automatically calculate the digital gain needed to overcome the dampening effect of your pads.
   * **Crosstalk Filter Dimensions:** Measure your physical build and update `PAD_WIDTH_CM`, `PAD_HEIGHT_CM`, and `SENSOR_DISTANCE_CM`. Accurate measurements allow the mathematical crosstalk filter to accurately reject sympathetic vibrations.
3. **Testing/Debugging Mode (Optional):** To test the raw analog input of a specific pad without using a DAW, remove the `//` before `#define DEBUG_TEST_PIN A6`. Upload the code, **close Hairless MIDI**, and open the Arduino Serial Monitor (set to 115200 baud). Remember to comment this line out (`//#define...`) before normal MIDI use.
4. Install the `LGT8fx Boards` package via Boards Manager. Select **LGT8F328** (Ensure Clock is set to **Internal 16 MHz** for clone boards).
5. Connect your hardware and click **Upload**. When routing through Hairless MIDI to your DAW, it is recommended to use ASIO drivers with a low buffer size (128-256smp) to eliminate audio processing delays.

---

<h2 id="tiếng-việt">🇻🇳 Tiếng Việt</h2>

Firmware cho dự án tự làm trống điện tử 8 mặt (OctaPad) dựa trên video gốc của [Share 4u](https://youtu.be/tHEJwT9Mr2s). Mã nguồn trong repository này được viết lại để tối ưu hóa cho vi điều khiển LGT8F328P (Type-C), đồng thời vẫn hỗ trợ tốt cho bo mạch Arduino Nano tiêu chuẩn.

### 🧰 Bảng Linh Kiện
| Linh kiện | Số lượng | Ghi chú |
| :--- | :---: | :--- |
| LGT8F328P (Type-C) / Arduino Nano | 1 | Vi điều khiển trung tâm |
| Cảm biến áp điện (Piezo 27mm/35mm) | 8 | Nhận diện lực gõ |
| Điện trở 1MΩ | 8 | Xả điện dung Piezo |
| Zener diode 5.1V | 8 | Cắt điện áp âm / Bảo vệ IC |
| Bo mạch đục lỗ (Perfboard) & Dây nối | 1 bộ | Gom mass chung và đi dây tín hiệu |
| Formex / Cao su xốp (Dày 1.0cm) | 8 | Làm mặt đệm gõ (Dampening) |

### ✨ Tính Năng
* Xử lý tín hiệu từ 8 cảm biến áp điện (Piezo) cùng lúc mà không yêu cầu sử dụng IC ghép kênh ngoại vi.
* Thuật toán bù trừ độ dày vật liệu tự động điều chỉnh dải ADC. Các mặt đệm dày (như formex 1.0cm) vẫn có thể đạt mức Velocity 127 của chuẩn MIDI mà không cần tác dụng lực vật lý quá lớn.
* Tích hợp ngắt âm thanh thủ công cho Crash Cymbal (Crash Choke) và nhận diện trạng thái đóng/mở của mặt Hi-Hat qua cảm biến pedal.
* Xuất tín hiệu Serial MIDI tuân thủ chuẩn General MIDI (GM), cho phép phần mềm DAW (như FPC trong FL Studio) nhận diện trực tiếp các mặt trống thông qua Hairless MIDI.

### 💻 Chi Tiết Kỹ Thuật
* Quy trình đọc cảm biến sử dụng cấu trúc Máy trạng thái (Finite State Machine) thay vì các vòng lặp chờ. Thời gian bắt đỉnh sóng được rút ngắn xuống mức 500µs, giúp giảm thiểu độ trễ tín hiệu.
* Hiện tượng nhiễu chéo (sympathetic vibration) lan truyền trên bề mặt được loại bỏ bằng thuật toán tính toán biên độ và khoảng cách vật lý giữa các cảm biến.
* Lỗi tràn biến đếm thời gian `micros()` (thường gây treo vi điều khiển sau 71 phút) đã được xử lý thông qua toán học số nguyên không dấu, đảm bảo hệ thống vận hành liên tục.
* Triệt tiêu điện dung dư trong cảm biến Piezo bằng cách chủ động kéo chân analog xuống mức thấp (`LOW`) trong pha nghỉ, giúp ngăn chặn lỗi nhận sai tín hiệu (double-trigger).

### 🚀 Hướng Dẫn Cài Đặt & Hiệu Chỉnh
1. Mở mã nguồn bằng phần mềm **Arduino IDE**.
2. Thiết lập các thông số vật lý trong code:
   * **Hiệu chuẩn độ nhạy (Boost):** Chỉnh sửa biến `MATERIAL_THICKNESS_CM` (ví dụ: `1.0`). Code sẽ tự động tính toán mức khuếch đại tín hiệu cần thiết để bù đắp lực cản của mặt formex/cao su.
   * **Kích thước lọc nhiễu:** Đo đạc hộp trống thực tế của bạn và điền vào `PAD_WIDTH_CM` (Rộng), `PAD_HEIGHT_CM` (Cao), và `SENSOR_DISTANCE_CM` (Khoảng cách giữa các cảm biến). Thông số này giúp thuật toán toán học triệt tiêu nhiễu chéo một cách chính xác nhất.
3. **Chế độ Test/Debug (Tùy chọn):** Để kiểm tra thông số thô của một chân cảm biến bất kỳ mà không cần DAW, hãy xóa dấu `//` trước dòng `#define DEBUG_TEST_PIN A6`. Nạp code vào mạch, **tắt hoàn toàn phần mềm Hairless MIDI**, sau đó mở Serial Monitor của Arduino (chọn mức 115200 baud). Lưu ý phải thêm lại dấu `//` trước khi chơi nhạc bình thường.
4. Cài đặt gói thư viện `LGT8fx Boards` trong mục Boards Manager. Chọn bo mạch **LGT8F328** (Cấu hình Clock bắt buộc là **Internal 16 MHz**). 
5. Cắm cáp kết nối và nhấn **Upload**. Khi sử dụng kèm các phần mềm DAW, nên thiết lập Audio driver là ASIO với mức Buffer Size thấp (128-256smp) để tránh độ trễ âm thanh từ máy tính.
