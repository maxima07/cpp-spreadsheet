    #include "sheet.h"

    #include "cell.h"
    #include "common.h"

    #include <algorithm>
    #include <functional>
    #include <iostream>
    #include <optional>

    using namespace std::literals;

    size_t PosHasher::operator()(const Position& pos) const {
        return pos_hasher_(pos.ToString());
    }

    Sheet::~Sheet() {}

    void Sheet::SetCell(Position pos, std::string text) {
        CheckPosition(pos);
        const auto& it = sheet_.find(pos);

        if (it == sheet_.end()) {
            sheet_[pos] = std::make_unique<Cell>(*this);
            CalcSheetSize (pos);
        } 
        
        sheet_.at(pos)->Set(std::move(text));
        
    }

    const CellInterface* Sheet::GetCell(Position pos) const {
        CheckPosition(pos);
        auto it = sheet_.find(pos);

        if (it != sheet_.end()) {
            return it->second.get();
        }

        return nullptr;
    }

    CellInterface* Sheet::GetCell(Position pos) {
        const Sheet* const_sheet = static_cast<const Sheet*>(this);
        return const_cast<CellInterface*>(const_sheet->GetCell(pos));
    }

    void Sheet::ClearCell(Position pos) {
        CheckPosition(pos);
        const auto& it = sheet_.find(pos);

        if (it != sheet_.end()) {
            it->second->Clear();
            sheet_.erase(it);
            sheet_size_ = {0, 0};

            for (const auto& [pos, _] : sheet_) {
                CalcSheetSize(pos);
            }
        }
    }

    Size Sheet::GetPrintableSize() const {
        if (!sheet_.empty()) {
            return sheet_size_;
        }
        return {0,0};
    }

    void Sheet::PrintValues(std::ostream& output) const {
        PrintCell(output, CellType::Value);
    }

    void Sheet::PrintTexts(std::ostream& output) const {
        PrintCell(output, CellType::Text);
    }

    std::unique_ptr<SheetInterface> CreateSheet() {
        return std::make_unique<Sheet>();
    }

    void Sheet::CalcSheetSize (const Position& change_pos) {
        sheet_size_.rows = sheet_size_.rows > change_pos.row ? sheet_size_.rows : change_pos.row + 1;
        sheet_size_.cols = sheet_size_.cols > change_pos.col ? sheet_size_.cols : change_pos.col + 1;
    }

    void Sheet::CheckPosition (const Position& pos) const {
        if (!pos.IsValid()) {
            throw InvalidPositionException ("Invalid Position"s);
        }
    }

    void Sheet::PrintCell (std::ostream& output, CellType type) const {
        Size size = GetPrintableSize();
        for (int row = 0; row < size.rows; ++row) {
            for (int col = 0; col < size.cols; ++col) {
                
                if (col > 0) {
                    output << "\t";
                }
                const auto& it = sheet_.find({row, col});
                
                if (it != sheet_.end()) {
                    switch (type) {
                        case CellType::Text : {
                            output << it->second->GetText();
                            break;
                        }
                        case CellType::Value : {
                            std::visit (
                                [&output](const auto value){
                                    output << value;
                                }, it->second->GetValue()
                            );
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            output << "\n";
        }
    }