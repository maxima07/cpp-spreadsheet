#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

using namespace std::literals;

/*=================Cell====================*/

Cell::Cell(SheetInterface& sheet)
    : impl_ (std::make_unique<EmptyImpl>())
    , sheet_ (sheet) {
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> tmp_impl;

    if (text.empty()) {
        tmp_impl = std::make_unique<EmptyImpl>();
    } else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        tmp_impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
        const Impl& expression = *tmp_impl;
        std::vector<Position> ref_cells_poitions = expression.GetReferencedCells();
        
        if (!ref_cells_poitions.empty() 
            && CheckCircularDependency (ref_cells_poitions)) {
                throw CircularDependencyException("The expression contains cyclic dependencies"s);
        }
        InvalidateCache ();
        UpdateDependencies (ref_cells_poitions);
    } else {
        tmp_impl = std::make_unique<TextImpl>(std::move(text));
    }
    impl_ = std::move(tmp_impl);
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
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
    
bool Cell::HasCache() const {
    return impl_->HasCache();
}

bool Cell::IsReferenced() const {
    return !dependent_cells_.empty();
}

void Cell::InvalidateCache() {
    if (HasCache()) {
        impl_->InvalidateCache();
        
        for (auto& cell : dependent_cells_) {
            cell->InvalidateCache();
        }
    }
}

bool Cell::CheckCircularDependency (const std::vector<Position>& referenced_cells) const{
    std::unordered_set<const Cell*> unique_cells_pos;
    std::unordered_set<const Cell*> processed_cells;
    std::vector<const Cell*> stack_process;

    for (const Position& pos : referenced_cells) {
        unique_cells_pos.insert(dynamic_cast<Cell*>(sheet_.GetCell(pos)));
    }

    stack_process.push_back(this);

    while (!stack_process.empty()) {
        const Cell* cell = stack_process.back();
        stack_process.pop_back();
        processed_cells.insert(cell);
        
        if (unique_cells_pos.find(cell) != unique_cells_pos.end()){
            return true;
        }

        for (const Cell* ref_cell : cell->referenced_cells_) {
            if (processed_cells.find(ref_cell) == processed_cells.end()) {
                stack_process.push_back(ref_cell);
            }
        }
    }
    return false;
}

void Cell::UpdateDependencies (std::vector<Position>& referenced_cells){
    for (auto& cell : dependent_cells_) {
        cell->dependent_cells_.erase(this);
    }

    referenced_cells_.clear();
    
    for (const auto& cell_pos : referenced_cells) {
        auto cell_ptr = dynamic_cast<Cell*>(sheet_.GetCell(cell_pos));
        
        if (!cell_ptr) {
            sheet_.SetCell(cell_pos, "");
            cell_ptr = dynamic_cast<Cell*>(sheet_.GetCell(cell_pos));
        }
        cell_ptr->dependent_cells_.insert(this);
        referenced_cells_.insert(cell_ptr);
    }
}

/*====================Impl=====================*/

void Cell::Impl::InvalidateCache() {}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}
        
bool Cell::Impl::HasCache() const {
    return true;
}

/*=================TextImpl====================*/

CellInterface::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}

/*=================TextImpl====================*/

Cell::TextImpl::TextImpl (std::string_view text) : text_(std::move(text)){}

CellInterface::Value Cell::TextImpl::GetValue() const {
    if (text_[0] == ESCAPE_SIGN) {
        return text_.substr(1);
    }
    return text_;
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

/*=================FormulaImpl====================*/

Cell::FormulaImpl::FormulaImpl (std::string_view expression, SheetInterface& sheet) 
    : formula_obj_ (ParseFormula(std::string(expression.substr(1)))) 
    , sheet_ (sheet){
}

CellInterface::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_) {
        cache_ = formula_obj_->Evaluate(sheet_);
    }
    
    if (std::holds_alternative<double>(cache_.value())) {
        return std::get<double>(cache_.value());
    }

    return std::get<FormulaError>(cache_.value());
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_obj_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_obj_->GetReferencedCells();
}

bool Cell::FormulaImpl::HasCache() {
    return cache_.has_value();
}

void Cell::FormulaImpl::InvalidateCache() {
    cache_.reset();
}

