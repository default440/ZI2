#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;

const int IP[64] = { 58,50,42,34,26,18,10,2,
                     60,52,44,36,28,20,12,4,
                     62,54,46,38,30,22,14,6,
                     64,56,48,40,32,24,16,8,
                     57,49,41,33,25,17, 9,1,
                     59,51,43,35,27,19,11,3,
                     61,53,45,37,29,21,13,5,
                     63,55,47,39,31,23,15,7 };

const int IP_1[64] = { 40,8,48,16,56,24,64,32,
                       39,7,47,15,55,23,63,31,
                       38,6,46,14,54,22,62,30,
                       37,5,45,13,53,21,61,29,
                       36,4,44,12,52,20,60,28,
                       35,3,43,11,51,19,59,27,
                       34,2,42,10,50,18,58,26,
                       33,1,41,9,49,17,57,25 };

const int E_exp[48] = { 32,1,2,3,4,5,
                        4,5,6,7,8,9,
                        8,9,10,11,12,13,
                        12,13,14,15,16,17,
                        16,17,18,19,20,21,
                        20,21,22,23,24,25,
                        24,25,26,27,28,29,
                        28,29,30,31,32,1 };

const int P_transform[32] = { 16,7,20,21,
                              29,12,28,17,
                              1,15,23,26,
                              5,18,31,10,
                              2,8,24,14,
                              32,27,3,9,
                              19,13,30,6,
                              22,11,4,25 };

const int PC_1[56] = { 57,49,41,33,25,17,9,
                       1,58,50,42,34,26,18,
                       10,2,59,51,43,35,27,
                       19,11,3,60,52,44,36,
                       63,55,47,39,31,23,15,
                       7,62,54,46,38,30,22,
                       14,6,61,53,45,37,29,
                       21,13,5,28,20,12,4 };

const int PC_2[56] = { 14,17,11,24,1,5,
                       3,28,15,6,21,10,
                       23,19,12,4,26,8,
                       16,7,27,20,13,2,
                       41,52,31,37,47,55,
                       30,40,51,45,33,48,
                       44,49,39,56,34,53,
                       46,42,50,36,29,32 };

const int SBOX_1[4][16] = { {14, 5, 7, 2, 11, 8, 1, 15, 0, 10, 9, 4, 6, 13, 12, 3},
                           {5, 0, 8, 15, 14, 3, 2, 12, 11, 7, 6, 9, 13, 4, 1, 10},
                           {4, 9, 2, 14, 8, 7, 13, 0, 10, 12, 15, 1, 5, 11, 3, 6},
                           {9, 6, 15, 5, 3, 8, 4, 11, 7, 1, 12, 2, 0, 14, 10, 13} };

typedef const int(*SBOX)[16];
SBOX SBox[8] = { SBOX_1, SBOX_1, SBOX_1, SBOX_1, SBOX_1, SBOX_1, SBOX_1, SBOX_1 };

class DES {
private:
    string total_code, total_key;
    string tempCode_1, tempCode_2;
    string m_code, code;
    string L, R, C, D;
    vector<string> K;
public:
    DES() {}
    void encode(string str, string key) {
        m_code = "";
        total_code = str;
        total_key = key;
        getKeys();
        formatSourceCode();
        int count = 0;
        int s = total_code.size();
        while (count * 8 < s) {
            string sub;
            if (s - count * 8 >= 8) {
                sub = total_code.substr(count * 8, 8);
            }
            else {
                sub = total_code.substr(count * 8);
            }
            count++;
            fill(sub);
            getIP0();
            string a = iterationT_A(1, 16);
            string result = getIP1(a);
            m_code += result;
        }
        cout << m_code << endl;
    }
    void decode(string str, string key) {
        int count = 0;
        code = "";
        getKeys();
        while (count * 64 < str.size()) {
            tempCode_1 = str.substr(count * 64, 64);
            total_key = key;
            count++;
            getIP0();
            string a = iterationT_D(16, 1);
            string result = getIP1(a);
            if (count * 64 == str.size()) {
                code += formatAndReduceResult(result);
            }
            else {
                code += formatResult(result);
            }
        }
        cout << code << endl;
    }

    ///============================================================================================================================

    // часть пароля
    // заполняем строку
    void fill(string str) {
        tempCode_1 = "";
        for (int i = 0; i < 8; i++) {
            string s;
            int a = i < str.size() ? (int)str[i] : 8 - str.size();
            while (a > 0) {
                s = (char)(a % 2 + 48) + s;
                a /= 2;
            }
            while (s.size() < 8) {
                s = "0" + s;
            }
            tempCode_1 += s;
        }
    }

    // заполняем строку
    void formatSourceCode() {
        if (total_code.size() % 8 == 0) {
            total_code += "\b\b\b\b\b\b\b\b";
        }
    }

    // Замена IP (LR)
    void getIP0() {
        tempCode_2 = tempCode_1;
        L = "";
        R = "";
        for (int i = 0; i < 64; i++) {
            tempCode_2[i] = tempCode_1[IP[i] - 1];
        }
        for (int i = 0; i < 64; i++) {
            if (i < 32) {
                L += tempCode_2[i];
            }
            else {
                R += tempCode_2[i];
            }
        }
    }

    // Круглая функция Фейстеля
    string Feistel(string R, string K) {
        string res = "", rec = "";
        // E-расширение строки Ri-1 до 32 бит
        string ER = E_expend(R);
        // E (Ri-1) и подключ Ki длиной 48 бит подвергаются операции XOR
        for (int i = 0; i < 48; i++) {
            res += (char)(((ER[i] - 48) ^ (K[i] - 48)) + 48);
        }
        // Равномерно разделить на 8 групп и пройти через 8 разных S-блоков для преобразования 6-4
        for (int i = 0; i < 8; i++) {
            string sub = res.substr(i * 6, 6);
            string sub_m = Feistel_SBOX(sub, i);
            // Подключаемся последовательно, чтобы получить строку длиной 32 бита
            rec += sub_m;
        }
        // P- перестановка
        return getPTransform(rec);
    }

    // P замена
    string getPTransform(string str) {
        string res = "";
        for (int i = 0; i < 32; i++) {
            res += str[P_transform[i] - 1];
        }
        return res;
    }

    // Feistel SBOX
    string Feistel_SBOX(string str, int num) {
        int n = (str[0] - 48) * 2 + (str[5] - 48);
        int m = (str[1] - 48) * 8 + (str[2] - 48) * 4 + (str[3] - 48) * 2 + (str[4] - 48);
        int number = SBox[num][n][m];
        string res = "";
        while (number > 0) {
            res = (char)(number % 2 + 48) + res;
            number /= 2;
        }
        while (res.size() < 4) {
            res = "0" + res;
        }
        return res;
    }

    // Расширение E
    string E_expend(string str) {
        string res = "";
        for (int i = 0; i < 48; i++) {
            res += str[E_exp[i] - 1];
        }
        return res;
    }

    // операция XOR
    string XORoperation(string a, string b) {
        string res = "";
        for (int i = 0; i < 32; i++) {
            res += (char)(((a[i] - 48) ^ (b[i] - 48)) + 48);
        }
        return res;
    }

    // Т итерация (зашифрованная)
    string iterationT_A(int begin, int end) {
        string L_temp, R_temp;
        for (int i = begin - 1; i <= end - 1; i++) {
            L_temp = R;
            R_temp = XORoperation(L, Feistel(R, K[i]));
            L = L_temp;
            R = R_temp;
        }
        return R + L;
    }

    // T итерация (расшифрована)
    string iterationT_D(int begin, int end) {
        string L_temp, R_temp;
        for (int i = begin - 1; i >= end - 1; i--) {
            L_temp = R;
            R_temp = XORoperation(L, Feistel(R, K[i]));
            L = L_temp;
            R = R_temp;
        }
        return R + L;
    }

    // Обратная замена IP
    string getIP1(string str) {
        string res = "";
        for (int i = 0; i < 64; i++) {
            res += str[IP_1[i] - 1];
        }
        return res;
    }

    // сортируем открытый текст 1
    string formatResult(string str) {
        int count = 0;
        string res = "";
        while (count * 8 < str.size()) {
            string a = str.substr(count * 8, 8);
            res += (char)(Two2Ten(a));
            count++;
        }
        return res;
    }

    // сортируем открытый текст 2
    string formatAndReduceResult(string str) {
        int count = 0;
        string res = "";
        string a = str.substr(str.size() - 8, 8);
        int n = (int)(Two2Ten(a));
        while (count < 8 - n) {
            a = str.substr(count * 8, 8);
            res += (char)(Two2Ten(a));
            count++;
        }
        return res;
    }

    // Двоичное в десятичное
    int Two2Ten(string num) {
        int base = 1;
        int res = 0;
        for (int i = num.size() - 1; i >= 0; i--) {
            res += (int)(num[i] - 48) * base;
            base *= 2;
        }
        return res;
    }

    //=============================================================================================================================

    // часть подраздела
    // Формат подключа
    string formatKey() {
        string res = "", rec = "";
        for (int i = 0; i < 8; i++) {
            int num = (int)total_key[i];
            res = "";
            while (num > 0) {
                res = (char)(num % 2 + 48) + res;
                num /= 2;
            }
            while (res.size() < 8) {
                res = "0" + res;
            }
            rec += res;
        }
        return rec;
    }
    // Замена PC1
    string getPC1Key(string str) {
        string res = str;
        for (int i = 0; i < 56; i++) {
            res[i] = str[PC_1[i] - 1];
        }
        return res;
    }
    // Получаем C, D
    void get_C_D(string str) {
        C = "";
        D = "";
        str.erase(63, 1);
        str.erase(55, 1);
        str.erase(47, 1);
        str.erase(39, 1);
        str.erase(31, 1);
        str.erase(23, 1);
        str.erase(15, 1);
        str.erase(7, 1);
        for (int i = 0; i < str.size(); i++) {
            if (i < 28) {
                C += str[i];
            }
            else {
                D += str[i];
            }
        }
    }
    // Замена LS
    void getKeyI() {
        //string C_temp = C, D_temp = D;
        for (int i = 1; i <= 16; i++) {
            if (i == 1 || i == 2 || i == 9 || i == 16) {
                LS_1(C);
                LS_1(D);
            }
            else {
                LS_2(C);
                LS_2(D);
            }
            string t = C + D;
            t = getPC2Key(t);
            K.push_back(t);
        }
    }
    // LS перестановка (1)
    void LS_1(string& str) {
        char a = str[0];
        for (int i = 0; i < str.size() - 1; i++) {
            str[i] = str[i + 1];
        }
        str[str.size() - 1] = a;
    }
    // LS перестановка (2)
    void LS_2(string& str) {
        char a = str[0], b = str[1];
        for (int i = 0; i < str.size() - 2; i++) {
            str[i] = str[i + 2];
        }
        str[str.size() - 2] = a;
        str[str.size() - 1] = b;
    }
    // Замена PC2
    string getPC2Key(string str) {
        string res = str;
        for (int i = 0; i < 48; i++) {
            res[i] = str[PC_2[i] - 1];
        }
        res.erase(53, 1);
        res.erase(42, 1);
        res.erase(37, 1);
        res.erase(34, 1);
        res.erase(24, 1);
        res.erase(21, 1);
        res.erase(17, 1);
        res.erase(8, 1);
        return res;
    }
    // Получаем общую функцию подраздела
    void getKeys() {
        vector<string> t;
        K = t;
        string a = formatKey();
        a = getPC1Key(a);
        get_C_D(a);
        getKeyI();
    }
};

int main() {
    string c, k;
    cout << "Plaintext: ";
    cin >> c;
    cout << "Key: ";
    cin >> k;
    DES des;
    cout << "result: ";
    des.encode(c, k);
    cout << endl;
    cout << "Ciphertext: ";
    cin >> c;
    cout << "Key: ";
    cin >> k;
    cout << "result: ";
    des.decode(c, k);
    return 0;
}
