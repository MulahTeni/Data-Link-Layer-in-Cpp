#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <cstdint>
#include <random>
#include <thread>
#include <chrono>

using namespace std;

// Ayarlar
constexpr size_t FRAME_SIZE = 100;
constexpr size_t CRC_SIZE = 16;
constexpr size_t TOTAL_SIZE = FRAME_SIZE + CRC_SIZE;

// CRC polinomu: x^16 + x^12 + x^5 + 1
const bitset<CRC_SIZE + 1> crcKey("10001000000100001");

// Stop&Wait hata oranları
constexpr double LOST_FRAME_PROB = 0.10;
constexpr double CORRUPTED_FRAME_PROB = 0.20;
constexpr double LOST_ACK_PROB = 0.15;
constexpr double CHECKSUM_ERROR_PROB = 0.05;

// Rastgele sayı üretici
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> prob(0.0, 1.0);

// Utils
void wait(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

vector<unsigned char> readDatFile(const string& filePath) {
    ifstream file(filePath, ios::binary);
    vector<unsigned char> data;
    if (!file) {
        cerr << "Dosya açılamadı: " << filePath << endl;
        return data;
    }
    unsigned char byte;
    while (file.read(reinterpret_cast<char*>(&byte), 1)) {
        data.push_back(byte);
    }
    return data;
}

vector<bool> bytesToBits(const vector<unsigned char>& data) {
    vector<bool> bits;
    for (unsigned char byte : data) {
        bitset<8> b(byte);
        for (int i = 7; i >= 0; --i) {
            bits.push_back(b[i]);
        }
    }
    return bits;
}

vector<vector<bool>> splitIntoFrames(const vector<bool>& bits) {
    vector<vector<bool>> frames;
    for (size_t i = 0; i < bits.size(); i += FRAME_SIZE) {
        vector<bool> frame(bits.begin() + i, bits.begin() + min(i + FRAME_SIZE, bits.size()));
        while (frame.size() < FRAME_SIZE) frame.push_back(0); // doldur
        frames.push_back(frame);
    }
    return frames;
}

vector<bitset<FRAME_SIZE>> toBitsets(const vector<vector<bool>>& frames) {
    vector<bitset<FRAME_SIZE>> result;
    for (const auto& frame : frames) {
        bitset<FRAME_SIZE> b;
        for (size_t i = 0; i < frame.size(); ++i) {
            b[FRAME_SIZE - 1 - i] = frame[i];
        }
        result.push_back(b);
    }
    return result;
}

bitset<CRC_SIZE> crcDivision(const bitset<CRC_SIZE>& bits, const bitset<CRC_SIZE + 1>& key) {
    bitset<CRC_SIZE> result;
    if (bits[0] == 0) {
        for (size_t i = 1; i <= CRC_SIZE; ++i)
            result[i - 1] = bits[i];
    } else {
        for (size_t i = 1; i <= CRC_SIZE; ++i)
            result[i - 1] = key[i] ^ bits[i];
    }
    return result;
}

vector<bitset<CRC_SIZE>> calculateCRC(const vector<bitset<FRAME_SIZE>>& frames) {
    vector<bitset<CRC_SIZE>> crcList;
    for (const auto& frame : frames) {
        bitset<TOTAL_SIZE> temp;
        for (size_t i = 0; i < FRAME_SIZE; ++i) {
            temp[TOTAL_SIZE - 1 - i] = frame[FRAME_SIZE - 1 - i];
        }

        bitset<CRC_SIZE> working;
        for (size_t i = 0; i < CRC_SIZE; ++i)
            working[i] = temp[i];

        for (size_t i = CRC_SIZE; i < TOTAL_SIZE; ++i) {
            working = crcDivision(working, crcKey);
            working <<= 1;
            working.set(0, temp[i]);
        }
        working = crcDivision(working, crcKey);
        crcList.push_back(working);
    }
    return crcList;
}

uint16_t calculateChecksum(const vector<uint16_t>& crcs) {
    uint32_t sum = 0;
    for (const auto& crc : crcs) {
        sum += crc;
        if (sum > 0xFFFF) sum = (sum & 0xFFFF) + 1;
    }
    uint16_t checksum = static_cast<uint16_t>(~sum);
    return checksum + 1;
}

// Simülasyon
void simulateTransmission(const vector<bitset<FRAME_SIZE>>& frames, const vector<bitset<CRC_SIZE>>& crcs, uint16_t checksum) {
    cout << "\n--- Veri Gönderimi Başladı (Stop&Wait) ---\n\n";
    for (size_t i = 0; i < frames.size(); ++i) {
        cout << "[Gönderici] Frame " << i << " gönderiliyor...\n";
        wait(500);

        double rand_val = prob(gen);
        if (rand_val < LOST_FRAME_PROB) {
            cout << "  🚫 Frame kayboldu! Timeout bekleniyor...\n\n";
            wait(500);
            --i; continue;
        }

        if (rand_val < LOST_FRAME_PROB + CORRUPTED_FRAME_PROB) {
            cout << "  ⚠️ Frame bozuldu! Alıcı CRC hatası algıladı. NACK gönderdi.\n\n";
            wait(500);
            --i; continue;
        }

        cout << "  ✅ Frame başarıyla alındı.\n";

        double ack_val = prob(gen);
        if (ack_val < LOST_ACK_PROB) {
            cout << "  🚫 ACK kayboldu. Timeout...\n\n";
            wait(500);
            --i; continue;
        }

        cout << "  ✅ ACK alındı. Bir sonraki frame'e geçiliyor.\n\n";
        wait(500);
    }

    cout << "--- Checksum Gönderimi ---\n";
    wait(500);
    double checksumError = prob(gen);
    if (checksumError < CHECKSUM_ERROR_PROB) {
        cout << "⚠️ Checksum bozuldu! Karşı taraf bütün veri setini reddetti.\n";
    } else {
        cout << "✅ Checksum başarıyla iletildi: 0x" << hex << uppercase << checksum << dec << "\n";
    }
    cout << "--- Gönderim Tamamlandı ---\n";
}

int main() {
    string path = "UDHR.dat";
    vector<unsigned char> rawData = readDatFile(path);
    if (rawData.empty()) return 1;

    vector<bool> bits = bytesToBits(rawData);
    auto boolFrames = splitIntoFrames(bits);
    auto frames = toBitsets(boolFrames);
    auto crcResults = calculateCRC(frames);

    vector<uint16_t> crcHex;
    for (const auto& crc : crcResults)
        crcHex.push_back(static_cast<uint16_t>(crc.to_ulong()));

    uint16_t checksum = calculateChecksum(crcHex);

    // Gösterim
    for (size_t i = 0; i < min(crcResults.size(), size_t(10)); ++i)
        cout << "Frame " << i << " CRC: " << crcResults[i] << endl;

    cout << "\nChecksum (2's complement): 0x" << hex << uppercase << checksum << dec << endl;

    // Simülasyon
    simulateTransmission(frames, crcResults, checksum);

    return 0;
}
