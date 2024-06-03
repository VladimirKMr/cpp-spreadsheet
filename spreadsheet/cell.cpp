#include "cell.h"


Cell::Cell(Sheet& sheet)
    : sheet_(sheet)
    , impl_(std::make_unique<EmptyImpl>()) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> new_impl;

    if (text.empty()) {
        new_impl = std::make_unique<EmptyImpl>();
    } else if (text.front() == FORMULA_SIGN && text.size() > 1) {
        new_impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    } else {
        new_impl = std::make_unique<TextImpl>(std::move(text));
    }

    if (HasCircularDependency(*new_impl)) {
        throw CircularDependencyException("Circular dependency detected");
    }

    for (const Position& pos : impl_->GetReferencedCells()) {
        Cell* ref_cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        if (ref_cell) {
            ref_cell->RemoveDependent(this);
        }
    }

    impl_ = std::move(new_impl);

    for (const Position& pos : impl_->GetReferencedCells()) {
        Cell* ref_cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        if (!ref_cell) {
            sheet_.SetCell(pos, "");
            ref_cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        }
        ref_cell->AddDependent(this);
    }

    InvalidateCache();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
    InvalidateCache();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !dependents_.empty();
}

bool Cell::HasCircularDependency(const Impl& new_impl) const {
    if (new_impl.GetReferencedCells().empty()) {
        return false;
    }

    std::unordered_set<const Cell*> referencies;
    for (const auto& pos : new_impl.GetReferencedCells()) {
        referencies.insert(dynamic_cast<Cell*>(sheet_.GetCell(pos)));
    }

    std::unordered_set<const Cell*> visited;
    std::stack<const Cell*> to_visit;
    to_visit.push(this);

    while (!to_visit.empty()) {
        const Cell* current = to_visit.top();
        to_visit.pop();
        visited.insert(current);

        if (referencies.find(current) != referencies.end()) { 
            return true;
        }

        for (const Cell* depend : current->dependents_) {
            if (visited.find(depend) == visited.end()) {
                to_visit.push(depend);
            }
        }
    }

    return false;
}

void Cell::InvalidateCache() {
    if (impl_->IsValidCache()) {
        impl_->InvalidateCache();
        for (Cell* dependent : dependents_) {
            dependent->InvalidateCache();
        }
    }   
}

void Cell::AddDependent(Cell* dependent) {
    dependents_.insert(dependent);
}

void Cell::RemoveDependent(Cell* dependent) {
    dependents_.erase(dependent);
}

// EmptyImpl
Cell::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
    return {};
}

// TextImpl
Cell::TextImpl::TextImpl(std::string text)
    : text_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {
    if (text_.front() == ESCAPE_SIGN) {
        return text_.substr(1);
    }
    return text_;
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
    return {};
}

// FormulaImpl
Cell::FormulaImpl::FormulaImpl(std::string text, const Sheet& sheet)
    : formula_(ParseFormula(std::move(text).substr(1))), sheet_(sheet) {}

Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cached_value_) {
        auto result = formula_->Evaluate(sheet_);
        cached_value_ = result;
    }

    if (std::holds_alternative<double>(*cached_value_)) {
        return std::get<double>(*cached_value_);
    } else {
        return std::get<FormulaError>(*cached_value_);
    }
}

std::string Cell::FormulaImpl::GetText() const {
    return "=" + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

bool Cell::FormulaImpl::IsValidCache() const {
    return cached_value_.has_value();
}

void Cell::FormulaImpl::InvalidateCache() {
    cached_value_.reset();
}