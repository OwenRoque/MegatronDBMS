#ifndef FRAME_H
#define FRAME_H

#include "page.h"

namespace Memory
{
    using frame_id_t = qint32;
    using page_id_t = Core::page_id_t;

    /**
     * @brief Parameter standart class, it allows implementation of another
     * policies which may require different parameters.
     * More parameters can be added here
     */
    struct UpdateParams
    {
        // default constructor
        UpdateParams() : page(nullptr), pinCount(0), dirtyBit(false), refBit(false) {}
        // members
        QSharedPointer<Core::Page> page;
        int pinCount;
        bool dirtyBit;
        bool refBit;
    };

    // General Frame Interface
    class Frame
    {
    public:
        /**
         * @brief Constructs a new Frame
         */
        Frame();

        /**
         * @brief Destroys a frame
         */
        virtual ~Frame() = default;

        /**
         *  @return the page object this frame holds
         */
        QSharedPointer<Core::Page> getPage() const;

        /**
         * @brief sets a new page to this frame, for cases when replacing free page objects to specific pages
         * @param page new page to replace
         */
        void setPage(QSharedPointer<Core::Page> page);

        /**
         *  @return true if the page in memory has been modified from the page on disk, false otherwise
         */
        bool isDirty() const;

        /**
         *  @return the pin count of this page
         */
        int getPinCount() const;

        /**
         * @brief set current frame's dirty value
         */
        void setDirty(bool value);

        /**
         * @brief increases the page pin count
         */
        void increasePinCount();

        /**
         * @brief decreases the page pin count
         */
        void decreasePinCount();

        /**
         * @brief updates frame metadata & content (page)
         * @param params parameter list
         */
        virtual void update(const UpdateParams& params) = 0;

    protected:
        // frame's data
        QSharedPointer<Core::Page> page;
        // flag to keep track of modified pages
        bool dirtyBit;
        // flag to keep record of number of users using the page
        int pinCount;

    };

    // LRU Frame: doesn't need any other metadata
    class LRUFrame : public Frame
    {
    public:
        /**
         * @brief Constructs a new LRUFrame
         */
        LRUFrame();

        /**
         * @brief update frame metadata & page
         * @param page new page to replace
         * @param pinCount new pin count value
         * @param dirtyBit default dirty bit state
         */
        void update(const UpdateParams& params) override;

    };

    // MRU Frame: doesn't need any other metadata
    class MRUFrame : public Frame
    {
    public:
        /**
         * @brief Constructs a new MRUFrame
         */
        MRUFrame();

        /**
         * @brief update frame metadata & page
         * @param page new page to replace
         * @param pinCount new pin count value
         * @param dirtyBit default dirty bit state
         */
        void update(const UpdateParams& params) override;

    };

    // Clock Frame: uses reference bit
    class ClockFrame : public Frame
    {
    public:
        /**
         * @brief Constructs a new Clock Frame
         */
        ClockFrame();

        /**
         * @brief update frame metadata & page
         * @param page new page to replace
         * @param pinCount new pin count value
         * @param dirtyBit default dirty bit state
         * @param refBit new refBit value
         */
        void update(const UpdateParams& params) override;

    private:
        bool refBit;

    };

    // LRU-K Frame: uses a list to store the history/timestamp access
    // TODO:
    // class LRUKFrame : public Frame
    // {
    // public:
    //     LRUKFrame() : Frame() {}
    //     void update(Core::Page* page, int pinCount, bool dirtyBit, const QList<int> &accessHistory);

    // private:
    //     QList<int> accessHistory;
    // };

}
#endif // FRAME_H
