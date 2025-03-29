#pragma once

#include "cell.h"
#include "common.h"

#include <map>
#include <functional>

struct PosHasher {
public:
    size_t operator()(const Position& pos) const;
private:
    std::hash<std::string> pos_hasher_;
};

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

	// Можете дополнить ваш класс нужными полями и методами

private:
	// Можете дополнить ваш класс нужными полями и 
    std::unordered_map<Position, std::unique_ptr<Cell>, PosHasher> sheet_;
    Size sheet_size_;

    void CalcSheetSize (const Position& pos);
    inline void CheckPosition (const Position& pos) const;
    inline void PrintCell (std::ostream& output, CellType type) const;
};