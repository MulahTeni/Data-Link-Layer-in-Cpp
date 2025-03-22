#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <cstdint>

using namespace std;

// Sabit uzunluklar
constexpr size_t FRAME_SIZE = 100;
constexpr size_t CRC_SIZE = 16;
constexpr size_t TOTAL_SIZE = FRAME_SIZE + CRC_SIZE;

// CRC polinomu: x^16 + x^12 + x^5 + 1
const bitset<CRC_SIZE + 1> crcKey("10001000000100001");

// Checksum Hesapla
uint16_t calculateChecksum(vector<uint16_t>& crcResultsHex) {
    uint32_t sum = 0;

    // Tüm CRC'leri first complementary arithmetic ile toplama
    for (const auto& crc : crcResultsHex) {
        sum += crc;

        // Wrap-around: taşma varsa en alta ekle
        if (sum > 0xFFFF) {
            sum = (sum & 0xFFFF) + 1;
        }
    }

    // Two's complement
    uint16_t checksum = static_cast<uint16_t>(~sum); 	// Ters çevirme // ~ bitwise NOT!
    checksum += 1;										// 1 ekleme

    return checksum;
}


// XOR algoritması
bitset<CRC_SIZE> crcDivision(const bitset<CRC_SIZE>& bits, const bitset<CRC_SIZE + 1>& crcKey) {
    bitset<CRC_SIZE> result_bits;

    if (bits[0] == 0) {
        for (size_t i = 1; i <= CRC_SIZE; ++i) {
            result_bits[i - 1] = 0 ^ bits[i];
        }
    } else {
        for (size_t i = 1; i <= CRC_SIZE; ++i) {
            result_bits[i - 1] = crcKey[i] ^ bits[i];
        }
    }

    return result_bits;
}

// CRC hesaplama
vector<bitset<CRC_SIZE>> calculateCRC(const vector<bitset<FRAME_SIZE>>& frames, const bitset<CRC_SIZE + 1>& crcKey) {
    vector<bitset<CRC_SIZE>> crc_results;

    for (const auto& frame : frames) {
        bitset<TOTAL_SIZE> tmp_frame;
        for (size_t i = 0; i < FRAME_SIZE; ++i) {
            tmp_frame[TOTAL_SIZE - 1 - i] = frame[FRAME_SIZE - 1 - i];
        }

        bitset<CRC_SIZE> tmp_bits;
        for (size_t i = 0; i < CRC_SIZE; ++i) {
            tmp_bits[i] = tmp_frame[i];
        }

        for (size_t i = CRC_SIZE; i < TOTAL_SIZE; ++i) {
            tmp_bits = crcDivision(tmp_bits, crcKey);
            tmp_bits <<= 1;
            tmp_bits.set(0, tmp_frame[i]);
        }

        tmp_bits = crcDivision(tmp_bits, crcKey);
        crc_results.push_back(tmp_bits);
    }

    return crc_results;
}

// vector<vector<bool>> -> vector<bitset<FRAME_SIZE>> dönüşümü
vector<bitset<FRAME_SIZE>> boolFramesTobitsetFrames(const vector<vector<bool>>& boolFrames) {
    vector<bitset<FRAME_SIZE>> bitsetFrames;
    for (const auto& frame : boolFrames) {
        bitset<FRAME_SIZE> b;
        for (size_t i = 0; i < frame.size(); ++i) {
            b[FRAME_SIZE - 1 - i] = frame[i];
        }
        bitsetFrames.push_back(b);
    }
    return bitsetFrames;
}

// Bit dizisini 100-bit'lik frame'lere böler
vector<vector<bool>> splitIntoFrames(const vector<bool>& bits, size_t frameSize = 100) {
    vector<vector<bool>> frames;
    size_t totalBits = bits.size();
    for (size_t i = 0; i < totalBits; i += frameSize) {
        size_t end = min(i + frameSize, totalBits);
        vector<bool> frame(bits.begin() + i, bits.begin() + end);
		
        // Eksikse sıfırla tamamla
        while (frame.size() < frameSize) {
            frame.push_back(0);
        }
        frames.push_back(frame);
    }
    return frames;
}

// Byte dizisini bit dizisine çevirir
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

// .dat dosyasını oku
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

    file.close();
    return data;
}

int main() {
    string path = "UDHR.dat";

    // Dosya oku
    vector<unsigned char> fileData = readDatFile(path);
    if (fileData.empty()) return 1;

    // Byte -> Bit
    vector<bool> allBits = bytesToBits(fileData);

    // Frame'lere böl
    auto boolFrames = splitIntoFrames(allBits);

    //cout << "Toplam " << boolFrames.size() << " frame üretildi." << endl;
    //cout << "İlk frame uzunluğu: " << boolFrames[0].size() << " bit" << endl;

    // Frame'leri bitset'e çevir
    vector<bitset<FRAME_SIZE>> bitsetFrames = boolFramesTobitsetFrames(boolFrames);

    // CRC hesapla
    auto crcResults = calculateCRC(bitsetFrames, crcKey);

	// İlk 10 sonucu yazdır
    for (size_t i = 0; i < min(crcResults.size(), size_t(10)); ++i) {
        cout << "Frame " << i << " CRC: " << crcResults[i] << endl;
    }
	
	vector<uint16_t> crcResultsHex;
	
	for (const auto& crc : crcResults) {
		crcResultsHex.push_back(static_cast<uint16_t>(crc.to_ulong()));
	}

	uint16_t checksum = calculateChecksum(crcResultsHex);

	cout << "Checksum (2's complement): 0x" << hex << uppercase << checksum << dec << endl;

	return 0;
}