#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"


class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl {
    public:
        virtual ~Impl() = default;
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual void InvalidateCache() = 0;
        virtual bool IsValidCache() const { return true; }
    };

    class EmptyImpl : public Impl {
    public:
        EmptyImpl() = default;
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        void InvalidateCache() override {}
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string text);
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        void InvalidateCache() override {}
    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string text, const Sheet& sheet);
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        void InvalidateCache() override;
        bool IsValidCache() const override;
    private:
        std::unique_ptr<FormulaInterface> formula_;
        const Sheet& sheet_;
        mutable std::optional<FormulaInterface::Value> cached_value_;
    };

    bool HasCircularDependency(const Impl& new_impl) const;
    void InvalidateCache();
    void AddDependent(Cell* dependent);
    void RemoveDependent(Cell* dependent);
    bool HasDependent(Cell* dependent) const;

    Sheet& sheet_;
    std::unique_ptr<Impl> impl_;
    std::unordered_set<Cell*> dependents_;
};