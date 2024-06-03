#include "formula.h"
#include "FormulaAST.h"


using namespace std::literals;

FormulaError::FormulaError(Category category) 
    : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case Category::Ref:
            return "#REF!";
        case Category::Value:
            return "#VALUE!";
        case Category::Arithmetic:
            return "#ARITHM!";
        }
    return "";
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
        try : ast_(ParseFormulaAST(expression)) 
    {
    }
        catch (const std::exception& e) {
            std::throw_with_nested(FormulaException(e.what()));
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        const std::function<double(Position)> args = [&sheet](const Position p)->double {
            if (!p.IsValid()) { 
                throw FormulaError(FormulaError::Category::Ref);
            }

            const auto* cell = sheet.GetCell(p);
            if (!cell) { 
                return 0;
            }

            if (std::holds_alternative<double>(cell->GetValue())) { 
                return std::get<double>(cell->GetValue());
            }

            if (std::holds_alternative<std::string>(cell->GetValue())) {
                auto value = std::get<std::string>(cell->GetValue());
                double result = 0;
                if (!value.empty()) {
                    std::istringstream in(value);
                    if (!(in >> result) || !in.eof()) { 
                        throw FormulaError(FormulaError::Category::Value);
                    }
                }
                return result;
            }

            throw FormulaError(std::get<FormulaError>(cell->GetValue()));
        };

        try {
            return ast_.Execute(args);
        } catch (FormulaError& fe) {
            return fe;
        }   
    }

    std::string GetExpression() const override {
        std::stringstream ss;
        ast_.PrintFormula(ss);
        return ss.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> cells;
        for (auto cell : ast_.GetCells()) {
            if (cell.IsValid()) { 
                cells.push_back(cell); 
            }
        }

        cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());
        return cells;
    }

private:
    const FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    } catch (const std::exception& exc) {
        std::throw_with_nested(FormulaException(exc.what()));
    }
}