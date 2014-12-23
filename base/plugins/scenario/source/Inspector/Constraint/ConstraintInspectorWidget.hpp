#pragma once

#include <InspectorInterface/InspectorWidgetBase.hpp>
#include <QMap>

class ConstraintModel;
class TemporalConstraintViewModel;
class BoxModel;
class DeckModel;
class ProcessSharedModelInterface;

class BoxWidget;
class BoxInspectorSection;
class QFormLayout;

/*!
 * \brief The ConstraintInspectorWidget class
 *
 * Inherits from InspectorWidgetInterface. Manages an inteface for an Constraint (Timebox) element.
 */
class ConstraintInspectorWidget : public InspectorWidgetBase
{
		Q_OBJECT
	public:
		explicit ConstraintInspectorWidget (TemporalConstraintViewModel* object, QWidget* parent = 0);

		TemporalConstraintViewModel* viewModel() const;
		ConstraintModel* model() const;

	public slots:
		void reloadDisplayedValues()
		{ updateDisplayedValues(m_currentConstraint); }
		void updateDisplayedValues(TemporalConstraintViewModel* obj);

		// These methods ask for creation
		void createProcess(QString processName);
		void createBox();

		// Interface of Constraint
		void on_processCreated(QString processName, int processId);
		void on_processRemoved(int processId);

		void on_boxCreated(int boxId);
		void on_boxRemoved(int boxId);

		// Abstract interface of SharedProcessModel

		// Abstract interface of ProcessViewModel

		// These methods are used to display created things
		void displaySharedProcess(ProcessSharedModelInterface*);
		void setupBox(BoxModel*);

	private:
		TemporalConstraintViewModel* m_currentConstraint{};
		QVector<QMetaObject::Connection> m_connections;

		InspectorSectionWidget* m_processSection{};
		std::vector<InspectorSectionWidget*> m_processesSectionWidgets;

		InspectorSectionWidget* m_boxSection{};
		BoxWidget* m_boxWidget{};
		QMap<int, BoxInspectorSection*> m_boxesSectionWidgets;

		std::vector<QWidget*> m_properties;

};
