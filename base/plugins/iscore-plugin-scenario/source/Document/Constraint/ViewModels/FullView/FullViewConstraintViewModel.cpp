#include "FullViewConstraintViewModel.hpp"

FullViewConstraintViewModel::FullViewConstraintViewModel(
        const id_type<ConstraintViewModel>& id,
        const ConstraintModel& model,
        QObject* parent) :
    ConstraintViewModel {id,
                                "FullViewConstraintViewModel",
                                model,
                                parent
}
{

}

FullViewConstraintViewModel* FullViewConstraintViewModel::clone(
        const id_type<ConstraintViewModel>& id,
        const ConstraintModel& cm,
        QObject* parent)
{
    auto cstr = new FullViewConstraintViewModel {id, cm, parent};
    cstr->showRack(this->shownRack());

    return cstr;
}
