#include "common.h"


const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

// Реализуйте методы:
bool Position::operator==(const Position rhs) const {
    return col == rhs.col && row == rhs.row;
}

bool Position::operator<(const Position rhs) const {
    return row < rhs.row || (row == rhs.row && col < rhs.col);
}

bool Position::IsValid() const {
    if (col < 0 || row < 0) {
        return false;
    }
    if (col >= MAX_COLS || row >= MAX_ROWS) {
        return false;
    }

    return true;
}

std::string Position::ToString() const {
    if (!this->IsValid()) {  // в случае невалидности возвращаем пустую строку
        return "";
    }

    std::string result;
    int col_num = col;
    
    while (col_num >= 0) {
        result.push_back('A' + col_num % LETTERS);
        col_num = col_num / LETTERS - 1;
    }

    std::reverse(result.begin(), result.end());
    result += std::to_string(row + 1);

    return result;
}

Position Position::FromString(std::string_view str) {
    // Поиск первой цифры
    size_t pos = 0;
    while (pos < str.size() && !isdigit(str[pos])) {
        ++pos;
    }

    // Обрезаем цифры (оставляем буквенные символы)
    std::string letters = std::string(str.substr(0, pos));

    // Записываем цифры (индекс строки)
    int row_result = -1;
    if (pos < str.size()) {
        std::string digits = std::string(str.substr(pos));
        if (!digits.empty() && digits.find_first_not_of("0123456789") == std::string::npos) {
            try {
                row_result = std::stoi(digits) - 1;
            } catch (const std::invalid_argument&) {
                return Position{-1, -1};
            } catch (const std::out_of_range&) {
                return Position{-1, -1};
            }
        }
    }

    // Вычисляем индекс столбца (из записанных символов в letters)
    int col_result = 0;
    if (!letters.empty()) {
        for (char c : letters) {
            if (c < 'A' || c > 'Z') {  // проверка на валидность символов (заглавные)
                return Position{-1, -1};
            }
            col_result = col_result * LETTERS + (c - 'A' + 1);
        }
        col_result--;
    } else {
        col_result = -1;
    }

    return Position{row_result, col_result};
}

bool Size::operator==(Size rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}