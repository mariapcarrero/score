#pragma once
#include <boost/optional/optional.hpp>
#include <QVariant>

#include <Curve/Segment/CurveSegmentFactoryKey.hpp>
#include <Curve/Segment/CurveSegmentModel.hpp>
#include <iscore/serialization/VisitorInterface.hpp>

class QObject;
struct CurveSegmentData;
#include <iscore/tools/SettableIdentifier.hpp>

struct ISCORE_PLUGIN_CURVE_EXPORT PowerCurveSegmentData
{
        static const CurveSegmentFactoryKey& key();
        double gamma;
};

Q_DECLARE_METATYPE(PowerCurveSegmentData)

class ISCORE_PLUGIN_CURVE_EXPORT PowerCurveSegmentModel final : public CurveSegmentModel
{
    public:
        using data_type = PowerCurveSegmentData;
        using CurveSegmentModel::CurveSegmentModel;
        PowerCurveSegmentModel(
                const CurveSegmentData& dat,
                QObject* parent);

        template<typename Impl>
        PowerCurveSegmentModel(Deserializer<Impl>& vis, QObject* parent) :
            CurveSegmentModel {vis, parent}
        {
            vis.writeTo(*this);
        }

        CurveSegmentModel* clone(
                const Id<CurveSegmentModel>& id,
                QObject* parent) const override;

        const CurveSegmentFactoryKey& key() const override;
        void serialize(const VisitorVariant& vis) const override;
        void on_startChanged() override;
        void on_endChanged() override;

        void updateData(int numInterp) const override;
        double valueAt(double x) const override;

        boost::optional<double> verticalParameter() const override;
        void setVerticalParameter(double p) override;

        QVariant toSegmentSpecificData() const override
        {
            return QVariant::fromValue(PowerCurveSegmentData{gamma});
        }

        double gamma = 12.05; // TODO private
};
