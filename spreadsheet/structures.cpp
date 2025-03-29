#include "common.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <tuple>

using namespace std::literals;

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

/*========================Position===============================*/

const Position Position::NONE = {-1, -1};

bool Position::operator==(const Position rhs) const {
    return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

/*
    bool IsValid() проверяет валидность позиции, то есть что ячейка (row, col) не выходит за
    ограничения ниже и что значения полей row и col неотрицательны. Position::NONE невалидна.
*/
bool Position::IsValid() const {
    return row >= 0 && col >= 0 && row < MAX_ROWS && col < MAX_COLS;
}

/*
    Возвращает строку — позицию в формате пользовательского индекса. Если позиция невалидна, 
    метод должен вернуть пустую строку.
*/


bool CheckStr (const std::string_view str) {
    if (str.empty() || str.size() > MAX_POSITION_LENGTH) {
        return false;
    }
    return true;
}

bool CheckLetters (std::string_view letters) {
    if (letters.empty() || letters.size() > MAX_POS_LETTER_COUNT) {
        return false;
    }
    return true;
}

bool CheckDigits (std::string_view digits) {
    if (digits.empty()) {
        return false;
    }

    for (const char ch : digits) {
        if (!std::isdigit(ch)){
            return false;
        }
    }

    return true;
}

std::string ConvColToStr (const int col) {
    std::string result = "";
    int c_num = col;

    while (c_num >= 0) {
        int let_in_abc = c_num % LETTERS;
        result.insert(result.begin(), char('A' + let_in_abc));
        c_num = c_num / LETTERS - 1;
    }

    return result;
}

int ConvStrToCol (const std::string_view letters) {
    int result = 0;

    for (char ch : letters) {
        int val = ch - 'A' + 1;
        result = result * LETTERS + val;
    }

    return result;
}

/*
    Возвращает строку — позицию в формате пользовательского индекса. Если позиция невалидна, 
    метод должен вернуть пустую строку.
*/
std::string Position::ToString() const {
    if (!IsValid()) {
        return "";
    }
    
    std::string result;
    result.reserve(MAX_POSITION_LENGTH);
    result += ConvColToStr(col);
    result += std::to_string(row + 1);
    return result;
}

/*
    Индекс ячейки для пользователя состоит из двух частей:
        - строка из заглавных букв латинского алфавита, обозначающая столбец;
        - число, обозначающее порядковый номер строки.
    Возвращает позицию, соответствующую индексу, заданному в str. Если индекс задан в неверном 
    формате — “abc”, “111”, “12jfd” — или выходит за предельные значения, он не валиден. Тогда 
    функция должна вернуть дефолтную позицию Position::NONE. Она объявлена в файле common.h и 
    определена в файле structures.cpp как {-1, -1}.
*/
Position Position::FromString(std::string_view str) {
    if (!CheckStr (str)) {
        return Position::NONE;
    }

    // Находим позицию с незаглавной буквой
    auto not_a_letter_ptr = std::find_if (str.begin(), str.end(), [](const char ch){
        return !(std::isalpha(ch) && std::isupper(ch));
    });

    // разделяем строку на буквы и цифры
    auto letters = str.substr(0, not_a_letter_ptr - str.begin());
    
    if (!CheckLetters(letters)) {
        return Position::NONE;
    }

    auto digits = str.substr (not_a_letter_ptr - str.begin());

    if (!CheckDigits(digits)) {
        return Position::NONE;
    }

    Position result;

    // Для пользователя строки нумеруются с 1, в программном представлении — с 0
    result.col = ConvStrToCol (letters) - 1;
    result.row = std::stoi(std::string(digits)) - 1;
    return result;
}

bool Size::operator==(Size rhs) const {
    return (this->rows == rhs.rows) && (this->cols == rhs.cols);
}

/*========================FormulaError===============================*/

FormulaError::FormulaError(Category category) : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case Category::Ref :
            return "#REF!"s;
            break;
        case Category::Value :
            return "#VALUE!"s;
            break;
        case Category::Arithmetic :
            return "#ARITHM!"s;
            break;
        default:
            return "";
            break;
        }
}

