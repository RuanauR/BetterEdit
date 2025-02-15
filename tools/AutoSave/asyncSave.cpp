#include "asyncSave.hpp"
#include "../RotateSaws/rotateSaws.hpp"
#include <thread>
#include <mutex>
#include "Backup/LevelBackupManager.hpp"

std::mutex g_stringMutex;
static std::string g_sharedString;

GDMAKE_HOOK(0x162480, "_ZN16LevelEditorLayer14getLevelStringEv")
std::string* __fastcall LevelEditorLayer_getLevelString(
    LevelEditorLayer* self, edx_t edx, std::string* resStr
) {
    if (!BetterEdit::getEnableAsyncSave())
        return GDMAKE_ORIG_P(self, edx, resStr);

    self->m_bHighDetail = false;

    g_sharedString = self->m_pLevelSettings->getSaveString() + ";";

    std::vector<std::thread> threads;

    CCARRAY_FOREACH_B_TYPE(self->m_pObjectContainerArrays, array, CCArray) {
        CCARRAY_FOREACH_B_TYPE(array, obj, GameObject) {
            if (!self->m_bHighDetail)
                self->m_bHighDetail = obj->m_bHighDetail;
            
            // structured bindings cant be captured sooo
            auto obj_ = obj;
            
            threads.push_back(std::thread ([obj_]() -> void {
                auto res = obj_->getSaveString();

                g_stringMutex.lock();

                // g_sharedString += res + ";";

                g_stringMutex.unlock();
            }));
        }
    }

    for (auto & t : threads)
        t.join();
    
    *resStr = g_sharedString;

    return resStr;
}

GDMAKE_HOOK(0x75010, "_ZN16EditorPauseLayer9saveLevelEv")
void __fastcall EditorPauseLayer_saveLevel(EditorPauseLayer* self) {
    if (shouldRotateSaw())
        stopRotations(self->m_pEditorLayer);
    
    if (self->m_pEditorLayer->m_ePlaybackMode != kPlaybackModeNot)
        self->m_pEditorLayer->m_pEditorUI->onStopPlaytest(nullptr);

    GDMAKE_ORIG_V(self);
    
    if (shouldRotateSaw())
        beginRotations(self->m_pEditorLayer);

    SoftSaveManager::clear();
    LevelBackupManager::get()->handleAutoBackupForLevel(
        self->m_pEditorLayer->m_pLevelSettings->m_pLevel
    );
}

