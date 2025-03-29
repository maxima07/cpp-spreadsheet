#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <unordered_set> 

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    
    bool HasCache() const;
    bool IsReferenced() const;
    void InvalidateCache();

private:
    class Impl {
    public:
        Impl () = default;
        virtual ~Impl () = default;

        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position>GetReferencedCells() const;
        
        virtual bool HasCache() const;
        virtual void InvalidateCache();
    };

    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl (std::string_view text);
        Value GetValue() const override;
        std::string GetText() const override;
    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl (std::string_view expression, SheetInterface& sheet);
        Value GetValue() const override;
        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;
        bool HasCache();
        void InvalidateCache();
    private:
        std::unique_ptr<FormulaInterface> formula_obj_;
        const SheetInterface& sheet_;
        mutable std::optional<FormulaInterface::Value> cache_;
    };

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
    std::unordered_set<Cell*> referenced_cells_;
    std::unordered_set<Cell*> dependent_cells_;

    bool CheckCircularDependency (const std::vector<Position>& referenced_cells) const;
    void UpdateDependencies (std::vector<Position>& referenced_cells);
    
};