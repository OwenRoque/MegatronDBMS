#ifndef MRUREPLACER_H
#define MRUREPLACER_H

#include "replacer.h"

namespace Memory
{
    class MRUReplacer : public Replacer
    {
    public:
        /**
         * @brief Create a new MRUReplacer
         * @param num_frames the maximum number of frames/pages the MRUReplacer will be required to store
         */
        explicit MRUReplacer(int num_frames);

        /**
         * @brief Destroys the MRUReplacer.
         */
        ~MRUReplacer() override;

        /**
         * @brief Remove the victim frame as defined by the replacement policy.
         * @param[out] frame_id id of frame that was removed, nullptr if no victim was found
         * @return true if a victim frame was found, false otherwise
         */
        auto victim(frame_id_t *frame_id) -> bool override;

        /**
         * @brief Pins a frame, indicating that it should not be victimized until it is unpinned.
         * @param frame_id the id of the frame to pin
         */
        void pin(frame_id_t frame_id) override;

        /**
         * @brief Unpins a frame, indicating that it can now be victimized.
         * @param frame_id the id of the frame to unpin
         */
        void unpin(frame_id_t frame_id) override;

        /** @return the number of elements in the replacer that can be victimized */
        auto size() -> size_t override;

    private:
        // maximum capacity of frame pool, same as pool size
        size_t numFrames;
        // implement list as a double ended queue to store 'victimizable' frames, with
        // the ascending time of reference from front to back
        QList<frame_id_t> frames;
        // to fetch the address of a key in the list quickly (find() takes O(N))
        QHash<frame_id_t, QList<frame_id_t>::iterator> frameMap;

    };
}


#endif // MRUREPLACER_H
