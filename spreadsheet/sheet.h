#pragma once

#include "cell.h"
#include "common.h"

template <>
struct std::hash<Position> {
    std::size_t operator()(const Position& pos) const {
        return std::hash<int>()(pos.row) ^ (std::hash<int>()(pos.col) << 1);
    }
};

class Cell;

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    
private:
    std::unordered_map<Position, std::unique_ptr<Cell>, std::hash<Position>> sheet_;
};