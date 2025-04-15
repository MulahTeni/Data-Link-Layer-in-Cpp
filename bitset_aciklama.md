
# `bitset.cpp` Açıklamalı İncelemesi

Bu belge, `bitset.cpp` dosyasındaki tüm kodun adım adım açıklamasını içerir. Dosya, bir `.dat` dosyasını okuyarak içeriğini bitlere dönüştürür, CRC (Cyclic Redundancy Check) hesaplaması yapar ve ardından bir checksum değeri üretir.

---

## 1. Sabitler ve Tanımlar

```cpp
constexpr size_t FRAME_SIZE = 100;
constexpr size_t CRC_SIZE = 16;
constexpr size_t TOTAL_SIZE = FRAME_SIZE + CRC_SIZE;
const bitset<CRC_SIZE + 1> crcKey("10001000000100001");
```
- **FRAME_SIZE:** Her veri bloğunun uzunluğu (bit cinsinden).
- **CRC_SIZE:** CRC sonucunun uzunluğu.
- **crcKey:** CRC polinomu. `x^16 + x^12 + x^5 + 1` polinomu ikilik olarak yazılmıştır.

---

## 2. Checksum Hesaplama

```cpp
uint16_t calculateChecksum(vector<uint16_t>& crcResultsHex)
```
- CRC sonuçlarını toplar.
- 16 bit sınırını aşarsa wrap-around işlemi yapar.
- İki'nin tümleyeni (2's complement) alarak checksum hesaplar.

---

## 3. XOR (CRC Bölme) Algoritması

```cpp
bitset<CRC_SIZE> crcDivision(...)
```
- CRC hesaplamasında kullanılan temel XOR işlemini gerçekleştirir.
- İlk bit 0 ise işlem `0` ile, değilse `crcKey` ile XOR yapılır.

---

## 4. CRC Hesaplama Fonksiyonu

```cpp
calculateCRC(...)
```
- Her 100 bitlik frame için CRC hesaplar.
- CRC işleminde frame'e 16 bitlik sıfır eklenerek CRC üretimi yapılır.

---

## 5. Frame İşleme Fonksiyonları

```cpp
splitIntoFrames(...)
```
- Bit dizisini 100 bitlik parçalara böler.

```cpp
boolFramesTobitsetFrames(...)
```
- `vector<vector<bool>>` tipindeki frame'leri `bitset<100>`'e çevirir.

---

## 6. Veri Okuma ve Dönüştürme

```cpp
bytesToBits(...)
```
- Byte dizisini bit dizisine çevirir.

```cpp
readDatFile(...)
```
- Belirtilen `.dat` dosyasını ikili olarak okur ve byte dizisine çevirir.

---

## 7. Ana Fonksiyon (main)

```cpp
int main()
```
- Dosya yolu tanımlanır: `"UDHR.dat"`
- Dosya okunur, bitlere çevrilir ve frame'lere ayrılır.
- CRC ve checksum hesaplanır.
- İlk 10 frame’in CRC sonucu ve genel checksum ekrana yazdırılır.

---

## 8. Çıktı Örneği

```text
Frame 0 CRC: 0101010101010101
Frame 1 CRC: 1100101010001110
...
Checksum (2's complement): 0x1A2B
```

---

## 9. Derleme ve Çalıştırma

```bash
g++ bitset.cpp -o bitset
./bitset
```

---

## Notlar

- `bitset` kullanımı ile doğrudan bit manipülasyonu yapılmıştır.
- Kodun tamamı `C++` ile yazılmış olup CRC ve checksum işlemleri için uygundur.

---

## 10. CRC Algoritması Detayı

CRC (Cyclic Redundancy Check) bir hata kontrol algoritmasıdır. Bu koddaki uygulama, veri üzerine CRC değeri ekleyerek hata kontrolü sağlar.

### Adımlar:
1. Her bir frame 100 bittir ve CRC eklenebilmesi için toplam 116 bitlik bir yapı oluşturulur (`TOTAL_SIZE`).
2. CRC hesaplaması için frame’in sonuna 16 sıfır bit eklenmiş gibi düşünülür.
3. CRC işlemi `crcDivision` fonksiyonuyla yapılır:
    - Her adımda 16 bitlik bir pencere alınır (`tmp_bits`).
    - Eğer en sol bit 1 ise, `crcKey` ile XOR yapılır. Aksi halde tüm bitler 0 ile XOR yapılır (değişmez).
    - Sonuç sola kaydırılır ve sıradaki bit eklenir.
    - Bu işlem frame’in sonuna kadar tekrar edilir.
    - En son kalan `tmp_bits` CRC sonucudur.

### crcDivision Fonksiyonu:

```cpp
bitset<CRC_SIZE> crcDivision(const bitset<CRC_SIZE>& bits, const bitset<CRC_SIZE + 1>& crcKey)
```
- `bits[0]` kontrol edilir.
- Eğer 0: tüm bitler `0 ^ bits[i]`
- Eğer 1: `crcKey[i] ^ bits[i]` yapılır.
- Sonuç yeni `tmp_bits`’e atanır.

---

## 11. Checksum Hesaplama Detayı

Checksum algoritması burada 16-bit’lik first complement arithmetic kullanır. Ardından 2'nin tümleyeni (two's complement) alınır.

### Adımlar:
1. Her CRC sonucu `uint16_t` olarak temsil edilir.
2. Hepsi 16-bit genişliğinde toplanır.
3. Toplam 16 bitten büyükse, taşan bitler en alta (wrap-around) eklenir:
   ```cpp
   if (sum > 0xFFFF) {
       sum = (sum & 0xFFFF) + 1;
   }
   ```
4. Bitwise NOT işlemi (`~sum`) ile tüm bitler ters çevrilir.
5. `+1` eklenerek two’s complement elde edilir.

### Neden two's complement?

Çünkü bu yöntem sayesinde alınan checksum ile alıcı tarafta tekrar toplama yapılabilir. Eğer hata yoksa sonuç `0xFFFF` olur ve bu da hata olup olmadığını anlamamızı sağlar.

### calculateChecksum Fonksiyonu:

```cpp
uint16_t calculateChecksum(vector<uint16_t>& crcResultsHex)
```
- `crcResultsHex` içerisindeki tüm değerleri toplar.
- Wrap-around düzeltmesi yapılır.
- Two’s complement alınarak `checksum` döndürülür.
