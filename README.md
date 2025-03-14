# ESPLORA3

ESPLORA3 adalah modul yang menggabungkan **ESP32** dan **LoRa** dalam satu board. Modul ini mendukung komunikasi **Point-to-Point (P2P)** serta **LoRaWAN**, sehingga cocok untuk berbagai aplikasi IoT dan jaringan nirkabel jarak jauh.

## Fitur Utama

- **Mikrokontroler**: ESP32 dengan dual-core dan konektivitas WiFi serta Bluetooth
- **Modul LoRa**: Mendukung komunikasi Long Range dengan protokol LoRaWAN dan P2P
- **Antena**: Konektor antena eksternal untuk sinyal optimal
- **Dukungan Pengembangan**: Kompatibel dengan platform Arduino, MicroPython, dan ESP-IDF

## Tampilan Board

### Tampak Atas
![Top View](Images/TOP.png)

### Tampak Bawah
![Bottom View](Images/BOTTOM.png)

## Penggunaan

1. **Konfigurasi Perangkat Keras**  
   - Hubungkan antena LoRa ke konektor yang tersedia.
   - Pastikan catu daya sesuai dengan spesifikasi.

2. **Pemrograman**  
   - Gunakan **Arduino IDE** atau **ESP-IDF** untuk mengembangkan firmware.
   - Pastikan library LoRa atau LoRaWAN sudah terinstal.

3. **Implementasi Jaringan**  
   - **Point-to-Point**: Gunakan dua board ESPLORA3 untuk komunikasi langsung.
   - **LoRaWAN**: Hubungkan ke gateway LoRaWAN untuk integrasi dengan jaringan LPWAN.

## Sumber Daya

- [Dokumentasi ESP32](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Panduan LoRa dan LoRaWAN](https://lora-alliance.org/about-lorawan/)

---

Untuk informasi lebih lanjut, silakan kunjungi repositori ini atau hubungi pengembang.

