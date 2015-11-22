#include <core/document/DocumentView.hpp>
#include <iscore/plugins/documentdelegate/DocumentDelegateViewInterface.hpp>
#include <iscore/plugins/documentdelegate/DocumentDelegateFactoryInterface.hpp>
#include <core/document/Document.hpp>

#include <QGridLayout>
using namespace iscore;

DocumentView::DocumentView(DocumentDelegateFactoryInterface* fact,
                           const Document& doc,
                           QWidget* parent) :
    QWidget {parent},
    m_document{doc},
    m_view{fact->makeView(m_document.context().app, this)}
{
    m_view->setParent(this);
    setObjectName("DocumentView");
    setLayout(new QGridLayout{this});
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(m_view->getWidget());
}
