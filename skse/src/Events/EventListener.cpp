#include "EventListener.h"

#include "Core/Core.h"
#include "Core/ThreadManager.h"
#include "GameAPI/Game.h"
#include "GameAPI/GameCamera.h"
#include "GameAPI/GameTable.h"
#include "Serial/Manager.h"
#include "UI/Align/AlignMenu.h"
#include "UI/UIState.h"
#include "Util/MathUtil.h"
#include "MCM/MCMTable.h"
#include "Util.h"

namespace Events {
    RE::BSEventNotifyControl EventListener::ProcessEvent(const RE::TESLoadGameEvent* a_event, RE::BSTEventSource<RE::TESLoadGameEvent>* a_eventSource) {
        logger::info("game loaded");

        Serialization::closeOldThreads();

        const auto skyrimVM = RE::SkyrimVM::GetSingleton();
        auto vm = skyrimVM ? skyrimVM->impl : nullptr;
        if (vm) {
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback(new CheckPapyrusUndressingCallbackFunctor());
            auto args = RE::MakeFunctionArguments();
            vm->DispatchStaticCall("OUndress", "UsePapyrusUndressing", args, callback);
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl EventListener::ProcessEvent(const SKSE::NiNodeUpdateEvent* a_event, RE::BSTEventSource<SKSE::NiNodeUpdateEvent>* a_eventSource) {
        if (a_event->reference->Is(RE::Actor::FORMTYPE)) {
            RE::Actor* actor = a_event->reference->As<RE::Actor>();
            OStim::ThreadActor* threadActor = OStim::ThreadManager::GetSingleton()->findActor(actor);
            if (threadActor) {
                threadActor->handleNiNodeUpdate();
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl EventListener::ProcessEvent(const SKSE::CrosshairRefEvent* a_event, RE::BSTEventSource<SKSE::CrosshairRefEvent>* a_eventSource) {
        if (a_event->crosshairRef) {
            // weird As call has to be made to make it non const
            GameAPI::GameTable::setCurrentCrosshairRef(a_event->crosshairRef->As<RE::TESObjectREFR>());
        } else {
            GameAPI::GameTable::setCurrentCrosshairRef(nullptr);
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl EventListener::ProcessEvent(RE::InputEvent* const* a_events, RE::BSTEventSource<RE::InputEvent*>* a_eventSource) {
        if (!a_events) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (RE::UI::GetSingleton()->GameIsPaused()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        for (RE::InputEvent* iEvent = *a_events; iEvent; iEvent = iEvent->next) {
            if (iEvent->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) {
                continue;
            }

            RE::ButtonEvent* bEvent = iEvent->AsButtonEvent();

            uint32_t keyCode;
            RE::INPUT_DEVICE deviceType = bEvent->GetDevice();
            uint32_t keyMask = bEvent->idCode;

            if (deviceType == RE::INPUT_DEVICE::kMouse) {
                keyCode = 256 + keyMask;
            } else if (deviceType == RE::INPUT_DEVICE::kGamepad) {
                keyCode = util::gamepadMaskToKeyCode(keyMask);
            } else {
                keyCode = keyMask;
            }

            if (keyCode == 0 || keyCode == 1 || keyCode >= 282) {
                // invalid key
                continue;
            }

            if (keyCode == MCM::MCMTable::keySceneStart()) {
                if (bEvent->IsDown()) {
                    GameAPI::GameActor player = GameAPI::GameActor::getPlayer();
                    GameAPI::GameActor target = GameAPI::Game::getCrosshairActor();
                    if (target) {
                        std::vector<GameAPI::GameActor> actors = GameAPI::GameActor::getPlayer().getNearbyActors(2000, [&player, &target](GameAPI::GameActor actor){
                            return actor != player && actor != target && OStim::isEligible(actor);    
                        });

                        if (actors.empty()) {
                            // TODO do this internally once we don't need OSA anymore
                            const auto skyrimVM = RE::SkyrimVM::GetSingleton();
                            auto vm = skyrimVM ? skyrimVM->impl : nullptr;
                            if (vm) {
                                RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
                                auto args = RE::MakeFunctionArguments<RE::Actor*, RE::Actor*, RE::Actor*>(std::move(RE::PlayerCharacter::GetSingleton()), std::move(target.form), std::move(nullptr));
                                vm->DispatchStaticCall("OSKSE", "StartScene", args, callback);
                            }
                        } else {
                            std::vector<std::string> options;

                            int max = std::min<int>(GameAPI::Game::getMessageBoxOptionLimit() - 1, actors.size());
                            int i = 0;
                            while (i < max) {
                                options.push_back(actors[i].getName());
                                i++;
                            }
                            options.push_back("none");

                            GameAPI::Game::showMessageBox("Add third actor?", options, [target, actors](unsigned int result) {
                                GameAPI::GameActor third;
                                if (result < actors.size() &&
                                    result != GameAPI::Game::getMessageBoxOptionLimit() - 1) {
                                    third = actors[result];
                                }

                                // TODO do this internally once we don't need OSA anymore
                                const auto skyrimVM = RE::SkyrimVM::GetSingleton();
                                auto vm = skyrimVM ? skyrimVM->impl : nullptr;
                                if (vm) {
                                    GameAPI::GameActor targetTemp = target;
                                    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
                                    auto args = RE::MakeFunctionArguments<RE::Actor*, RE::Actor*, RE::Actor*>(std::move(RE::PlayerCharacter::GetSingleton()), std::move(targetTemp.form), std::move(third.form));
                                    vm->DispatchStaticCall("OSKSE", "StartScene", args, callback);
                                }
                            });
                        }

                        
                    }
                } else if (bEvent->IsUp() && bEvent->HeldDuration() >= 2.0) {
                    if (!OStim::ThreadManager::GetSingleton()->playerThreadRunning()) {
                        // TODO do this internally once we don't need OSA anymore
                        const auto skyrimVM = RE::SkyrimVM::GetSingleton();
                        auto vm = skyrimVM ? skyrimVM->impl : nullptr;
                        if (vm) {
                            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
                            auto args = RE::MakeFunctionArguments<RE::Actor*>(std::move(RE::PlayerCharacter::GetSingleton()));
                            vm->DispatchStaticCall("OSKSE", "Masturbate", args, callback);
                        }
                    }
                }
            }

            if (!bEvent->IsDown()) {
                continue;
            }

            if (!OStim::ThreadManager::GetSingleton()->playerThreadRunning()) {
                continue;
            }

            // TODO do this internally once we don't need OSA anymore
            const auto skyrimVM = RE::SkyrimVM::GetSingleton();
            auto vm = skyrimVM ? skyrimVM->impl : nullptr;
            if (vm) {
                RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
                auto args = RE::MakeFunctionArguments();
                vm->DispatchStaticCall("OSKSE", "ShowBars", args, callback);
            }

            if (keyCode == MCM::MCMTable::keyUp()) {
                UI::UIState::GetSingleton()->HandleControl(UI::Controls::Up);
            } else if (keyCode == MCM::MCMTable::keyDown()) {
                UI::UIState::GetSingleton()->HandleControl(UI::Controls::Down);
            } else if (keyCode == MCM::MCMTable::keyLeft()) {
                UI::UIState::GetSingleton()->HandleControl(UI::Controls::Left);
            } else if (keyCode == MCM::MCMTable::keyRight()) {
                UI::UIState::GetSingleton()->HandleControl(UI::Controls::Right);
            } else if (keyCode == MCM::MCMTable::keyYes()) {
                UI::UIState::GetSingleton()->HandleControl(UI::Controls::Yes);
            } else if (keyCode == MCM::MCMTable::keyToggle()) {
                UI::UIState::GetSingleton()->HandleControl(UI::Controls::Toggle);
            } else if (keyCode == MCM::MCMTable::keyEnd()) {
                OStim::ThreadManager::GetSingleton()->getPlayerThread()->stop();
            } else if (keyCode == MCM::MCMTable::keySpeedUp()) {
                OStim::ThreadManager::GetSingleton()->getPlayerThread()->increaseSpeed();
            } else if (keyCode == MCM::MCMTable::keySpeedDown()) {
                OStim::ThreadManager::GetSingleton()->getPlayerThread()->decreaseSpeed();
            } else if (keyCode == MCM::MCMTable::keyPullOut()){
                if (UI::UIState::GetSingleton()->GetActiveMenu() == UI::MenuType::kAlignMenu) {
                    UI::UIState::GetSingleton()->HandleControl(UI::Controls::No);
                } else {
                    // TODO do this internally once we don't need OSA anymore
                    if (vm) {
                        RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
                        auto args = RE::MakeFunctionArguments();
                        vm->DispatchStaticCall("OSKSE", "PullOut", args, callback);
                    }
                }
            } else if (keyCode == MCM::MCMTable::keyAutoMode()){
                OStim::Thread* thread = OStim::ThreadManager::GetSingleton()->getPlayerThread();
                if (thread) {
                    if (thread->isInAutoMode()) {
                        thread->stopAutoMode();
                    } else {
                        thread->startAutoMode();
                    }
                }
            } else if (keyCode == MCM::MCMTable::keyAlignment()) {
                auto uiState = UI::UIState::GetSingleton();
                uiState->SwitchActiveMenu(uiState->GetActiveMenu() == UI::MenuType::kAlignMenu ? UI::MenuType::kSceneMenu : UI::MenuType::kAlignMenu);
            } else if (keyCode == MCM::MCMTable::keyFreeCam()) {
                GameAPI::GameCamera::toggleFreeCam();
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
}