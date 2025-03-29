#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) 
        try : ast_(ParseFormulaAST (std::move(expression))){
        } catch (...) {
            throw FormulaException ("Syntactically incorrect formula"s);
        }

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute (sheet);
        } catch (const FormulaError& evaluate_err) {
            return evaluate_err;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream ostr;
        ast_.PrintFormula (ostr);
        return ostr.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> referenced_cells;
        
        for (const auto& cell : ast_.GetCells()) {
            if (cell.IsValid()) {
                referenced_cells.push_back(cell);
            } else {
                continue;
            }
        }
        return referenced_cells;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}