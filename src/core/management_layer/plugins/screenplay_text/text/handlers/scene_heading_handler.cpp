#include "scene_heading_handler.h"

#include "../screenplay_text_edit.h"

#include <business_layer/model/locations/locations_model.h>
#include <business_layer/model/screenplay/screenplay_dictionaries_model.h>
#include <business_layer/model/screenplay/text/screenplay_text_block_parser.h>
#include <business_layer/templates/screenplay_template.h>

#include <QKeyEvent>
#include <QStringListModel>
#include <QTextBlock>
#include <QTimer>

using BusinessLayer::SceneHeadingParser;
using BusinessLayer::ScreenplayParagraphType;
using Ui::ScreenplayTextEdit;


namespace KeyProcessingLayer {

SceneHeadingHandler::SceneHeadingHandler(Ui::ScreenplayTextEdit* _editor)
    : StandardKeyHandler(_editor)
    , m_completerModel(new QStringListModel(_editor))
{
}

void SceneHeadingHandler::handleEnter(QKeyEvent* _event)
{
    //
    // Получим необходимые значения
    //
    // ... курсор в текущем положении
    QTextCursor cursor = editor()->textCursor();
    // ... блок текста в котором находится курсор
    QTextBlock currentBlock = cursor.block();
    // ... текст до курсора
    QString cursorBackwardText = currentBlock.text().left(cursor.positionInBlock());
    // ... текст после курсора
    QString cursorForwardText = currentBlock.text().mid(cursor.positionInBlock());
    // ... текущая секция
    SceneHeadingParser::Section currentSection = SceneHeadingParser::section(cursorBackwardText);


    //
    // Обработка
    //
    if (editor()->isCompleterVisible()) {
        //! Если открыт подстановщик

        //
        // Вставить выбранный вариант
        //
        editor()->applyCompletion();

        //
        // Обновим курсор, т.к. после автозавершения он смещается
        //
        cursor = editor()->textCursor();

        //
        // Дописать необходимые символы
        //
        switch (currentSection) {
        case SceneHeadingParser::SectionSceneIntro: {
            cursor.insertText(" ");
            break;
        }

        default: {
            break;
        }
        }

        //
        // Покажем подсказку, если это возможно
        //
        handleOther();

        //
        // Если нужно автоматически перепрыгиваем к следующему блоку
        //
        if (_event != nullptr // ... чтобы таб не переводил на новую строку
            && currentSection == SceneHeadingParser::SectionSceneTime) {
            //
            // Сохраним параметры сцены
            //
            storeSceneParameters();
            //
            // Переходим к следующему блоку
            //
            cursor.movePosition(QTextCursor::EndOfBlock);
            editor()->setTextCursor(cursor);
            editor()->addParagraph(jumpForEnter(ScreenplayParagraphType::SceneHeading));
        }
    } else {
        //! Подстановщик закрыт

        if (cursor.hasSelection()) {
            //! Есть выделение

            //
            // Удаляем всё, но оставляем стилем блока текущий
            //
            editor()->addParagraph(ScreenplayParagraphType::SceneHeading);
        } else {
            //! Нет выделения

            if (cursorBackwardText.isEmpty() && cursorForwardText.isEmpty()) {
                //! Текст пуст

                //
                // Меняем в соответствии с настройками
                //
                editor()->setCurrentParagraphType(
                    changeForEnter(ScreenplayParagraphType::SceneHeading));
            } else {
                //! Текст не пуст

                //
                // Сохраним параметры сцены
                //
                storeSceneParameters();

                if (cursorBackwardText.isEmpty()) {
                    //! В начале блока

                    //
                    // Вставка блока заголовка перед собой
                    //
                    editor()->addParagraph(ScreenplayParagraphType::SceneHeading);
                } else if (cursorForwardText.isEmpty()) {
                    //! В конце блока

                    //
                    // Вставка блока описания действия
                    //
                    editor()->addParagraph(jumpForEnter(ScreenplayParagraphType::SceneHeading));
                } else {
                    //! Внутри блока

                    //
                    // Вставка блока описания действия
                    //
                    editor()->addParagraph(ScreenplayParagraphType::Action);
                }
            }
        }
    }
}

void SceneHeadingHandler::handleTab(QKeyEvent*)
{
    //
    // Получим необходимые значения
    //
    // ... курсор в текущем положении
    QTextCursor cursor = editor()->textCursor();
    // ... блок текста в котором находится курсор
    QTextBlock currentBlock = cursor.block();
    // ... текст до курсора
    QString cursorBackwardText = currentBlock.text().left(cursor.positionInBlock());
    // ... текст после курсора
    QString cursorForwardText = currentBlock.text().mid(cursor.positionInBlock());


    //
    // Обработка
    //
    if (editor()->isCompleterVisible()) {
        //! Если открыт подстановщик

        //
        // Работаем аналогично нажатию ENTER
        //
        handleEnter();
    } else {
        //! Подстановщик закрыт

        if (cursor.hasSelection()) {
            //! Есть выделение

            //
            // Ни чего не делаем
            //
        } else {
            //! Нет выделения

            if (cursorBackwardText.isEmpty() && cursorForwardText.isEmpty()) {
                //! Текст пуст

                //
                // Если строка пуста, то сменить стиль на описание действия
                //
                editor()->setCurrentParagraphType(
                    changeForTab(ScreenplayParagraphType::SceneHeading));
            } else {
                //! Текст не пуст

                if (cursorBackwardText.isEmpty()) {
                    //! В начале блока

                    //
                    // Ни чего не делаем
                    //
                } else if (cursorForwardText.isEmpty()) {
                    //! В конце блока

                    //
                    // Если в секции локации, то добавление " - " и отображение подсказки
                    //
                    if (SceneHeadingParser::section(cursorBackwardText)
                        == SceneHeadingParser::SectionLocation) {
                        //
                        // Добавим необходимый текст в зависимости от того, что ввёл пользователь
                        //
                        if (cursorBackwardText.endsWith(" -")) {
                            cursor.insertText(" ");
                        } else if (cursorBackwardText.endsWith(" ")) {
                            cursor.insertText("- ");
                        } else {
                            cursor.insertText(" - ");
                        }

                        //
                        // Отображение подсказки
                        //
                        handleOther();
                    }

                    //
                    // В противном случае перейдём к блоку участников сцены
                    //
                    else {
                        //
                        // Сохраним параметры сцены
                        //
                        storeSceneParameters();

                        //
                        // А затем вставим блок
                        //
                        editor()->addParagraph(jumpForTab(ScreenplayParagraphType::SceneHeading));
                    }
                } else {
                    //! Внутри блока

                    //
                    // Ни чего не делаем
                    //
                }
            }
        }
    }
}

void SceneHeadingHandler::handleOther(QKeyEvent*)
{
    //
    // Получим необходимые значения
    //
    // ... курсор в текущем положении
    QTextCursor cursor = editor()->textCursor();
    // ... блок текста в котором находится курсор
    QTextBlock currentBlock = cursor.block();
    // ... текст блока
    QString currentBlockText = currentBlock.text();
    // ... текст до курсора
    QString cursorBackwardText = currentBlockText.left(cursor.positionInBlock());

    //
    // Покажем подсказку, если это возможно
    //
    complete(currentBlockText, cursorBackwardText);
}

void SceneHeadingHandler::handleInput(QInputMethodEvent* _event)
{
    Q_UNUSED(_event)

    //
    // Получим необходимые значения
    //
    // ... курсор в текущем положении
    const QTextCursor cursor = editor()->textCursor();
    int cursorPosition = cursor.positionInBlock();
    // ... блок текста в котором находится курсор
    const QTextBlock currentBlock = cursor.block();
    // ... текст блока
    QString currentBlockText = currentBlock.text();
    // ... текст до курсора
    const QString cursorBackwardText = currentBlockText.left(cursorPosition);

    //
    // Покажем подсказку, если это возможно
    //
    complete(currentBlockText, cursorBackwardText);
}

void SceneHeadingHandler::complete(const QString& _currentBlockText,
                                   const QString& _cursorBackwardText)
{
    qDebug("comp");
    //
    // Текущая секция
    //
    SceneHeadingParser::Section currentSection = SceneHeadingParser::section(_cursorBackwardText);

    //
    // Получим модель подсказок для текущей секции и выведем пользователю
    //
    QAbstractItemModel* sectionModel = nullptr;
    //
    // ... в соответствии со введённым в секции текстом
    //
    QString sectionText;

    switch (currentSection) {
    case SceneHeadingParser::SectionSceneIntro: {
        m_completerModel->setStringList(editor()->dictionaries()->sceneIntros().toList());
        sectionModel = m_completerModel;
        sectionText = SceneHeadingParser::sceneIntro(_currentBlockText);
        break;
    }

    case SceneHeadingParser::SectionLocation: {
        sectionModel = editor()->locations();
        sectionText = SceneHeadingParser::location(_currentBlockText);
        break;
    }

    case SceneHeadingParser::SectionStoryDay: {
        m_completerModel->setStringList(editor()->dictionaries()->storyDays().toList());
        sectionModel = m_completerModel;
        sectionText = SceneHeadingParser::storyDay(_currentBlockText);
        break;
    }

    case SceneHeadingParser::SectionSceneTime: {
        //
        // Возможно пользователь предпочитает обозначать локации и подлокации через минус,
        // поэтому проверяем нет ли уже сохранённых локаций такого рода, и если есть, и они
        // подходят под дополнение, то используем их
        //
        bool useLocations = false;
        const bool force = true;
        const QString locationFromBlock = SceneHeadingParser::location(_currentBlockText, force);
        const auto locationsModel = editor()->locations();
        for (int locationRow = 0; locationRow < locationsModel->rowCount(); ++locationRow) {
            const auto location
                = locationsModel->data(locationsModel->index(locationRow, 0), Qt::DisplayRole)
                      .toString();
            if (location.startsWith(locationFromBlock, Qt::CaseInsensitive)) {
                useLocations = true;
                break;
            }
        }
        if (useLocations) {
            sectionModel = locationsModel;
            sectionText = locationFromBlock;
        }
        //
        // Во всех остальных случаях используем дополнение по времени действия
        //
        else {
            m_completerModel->setStringList(editor()->dictionaries()->sceneTimes().toList());
            sectionModel = m_completerModel;
            sectionText = SceneHeadingParser::sceneTime(_currentBlockText);
        }
        break;
    }

    default: {
        break;
    }
    }

    //
    // Дополним текст
    //
    int cursorMovement = sectionText.length();
    while (!_cursorBackwardText.endsWith(sectionText.left(cursorMovement), Qt::CaseInsensitive)) {
        --cursorMovement;
    }
    //
    // ... дополняем, когда цикл обработки событий выполнится, чтобы позиция курсора
    //     корректно определилась после изменения текста
    //
    QTimer::singleShot(0, [this, sectionModel, sectionText, cursorMovement] {
        editor()->complete(sectionModel, sectionText, cursorMovement);
    });
}

void SceneHeadingHandler::storeSceneParameters() const
{
    //
    // Получим необходимые значения
    //
    // ... курсор в текущем положении
    const QTextCursor cursor = editor()->textCursor();
    // ... блок текста в котором находится курсор
    const QTextBlock currentBlock = cursor.block();
    // ... текст блока
    const QString currentBlockText = currentBlock.text();
    // ... текст до курсора
    const QString cursorBackwardText = currentBlockText.left(cursor.positionInBlock());

    //
    // Сохраняем время
    //
    const QString sceneIntro = SceneHeadingParser::sceneIntro(cursorBackwardText);
    editor()->dictionaries()->addSceneIntro(sceneIntro);

    //
    // Сохраняем локацию
    //
    const QString location = SceneHeadingParser::location(cursorBackwardText);
    editor()->locations()->createLocation(location);

    //
    // Сохраняем место
    //
    const QString sceneTime = SceneHeadingParser::sceneTime(cursorBackwardText);
    editor()->dictionaries()->addSceneTime(sceneTime);

    //
    // Сохраняем сценарный день
    //
    const QString storyDay = SceneHeadingParser::storyDay(cursorBackwardText);
    editor()->dictionaries()->addStoryDay(storyDay);
}

} // namespace KeyProcessingLayer
