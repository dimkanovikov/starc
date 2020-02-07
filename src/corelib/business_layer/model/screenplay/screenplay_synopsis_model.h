#pragma once

#include "../abstract_model.h"


namespace BusinessLayer
{

/**
 * @brief Модель
 */
class ScreenplaySynopsisModel : public AbstractModel
{
    Q_OBJECT

public:
    explicit ScreenplaySynopsisModel(QObject* _parent = nullptr);

protected:
    /**
     * @brief Реализация модели для работы с документами
     */
    /** @{ */
    void initDocument() override;
    void clearDocument() override;
    QByteArray toXml() const override;
    /** @} */
};

} // namespace BusinessLayer