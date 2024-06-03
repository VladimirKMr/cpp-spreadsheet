#include "cell.h"
#include "sheet.h"
#include "common.h"

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Incorrect position");
    }

    if (sheet_.find(pos) == sheet_.end()) {
        sheet_[pos] = std::make_unique<Cell>(*this);
    }
    sheet_[pos]->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Incorrect position");
    }

    const auto it = sheet_.find(pos);
    if (it == sheet_.end()) {
        return nullptr;
    }
    return sheet_.at(pos).get();
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Incorrect position");
    }

    auto it = sheet_.find(pos);
    if (it == sheet_.end()) {
        return nullptr;
    }
    return sheet_.at(pos).get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Incorrect position");
    }

    if (sheet_.find(pos) != sheet_.end()) {
        sheet_.erase(pos);
    }
}

Size Sheet::GetPrintableSize() const {
    int max_row = 0;
    int max_col = 0;

    for (const auto& [pos, cell] : sheet_) {
        if (!cell->GetText().empty()) {
            max_row = std::max(max_row, pos.row + 1);
            max_col = std::max(max_col, pos.col + 1);
        }
    }

    return {max_row, max_col};
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();

    for (int row = 0; row < size.rows; ++row) {
        bool first = true;
        for (int col = 0; col < size.cols; ++col) {
            if (!first) {
                output << '\t';
            } else {
                first = false;
            }

            auto it = sheet_.find({row, col});
            if (it != sheet_.end()) {
                auto value = it->second->GetValue();
                std::visit([&output](const auto& x) { output << x; }, value);
            } else {
                output << "";
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        bool first = true;
        for (int col = 0; col < size.cols; ++col) {
            if (!first) {
                output << '\t';
            } else {
                first = false;
            }

            auto it = sheet_.find({row, col});
            if (it != sheet_.end()) {
                output << it->second->GetText();
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}