// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/extensions/extension_keybinding_registry_views.h"

#include "chrome/browser/extensions/api/commands/command_service.h"
#include "chrome/browser/extensions/extension_keybinding_registry.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/common/extension.h"
#include "ui/views/focus/focus_manager.h"

// static
void extensions::ExtensionKeybindingRegistry::SetShortcutHandlingSuspended(
    bool suspended) {
  views::FocusManager::set_shortcut_handling_suspended(suspended);
}

ExtensionKeybindingRegistryViews::ExtensionKeybindingRegistryViews(
    Profile* profile,
    views::FocusManager* focus_manager,
    ExtensionFilter extension_filter,
    Delegate* delegate)
    : ExtensionKeybindingRegistry(profile, extension_filter, delegate),
      profile_(profile),
      focus_manager_(focus_manager) {
  Init();
}

ExtensionKeybindingRegistryViews::~ExtensionKeybindingRegistryViews() {
  for (EventTargets::const_iterator iter = event_targets_.begin();
       iter != event_targets_.end(); ++iter)
    focus_manager_->UnregisterAccelerator(iter->first, this);
}

void ExtensionKeybindingRegistryViews::AddExtensionKeybinding(
    const extensions::Extension* extension,
    const std::string& command_name) {
  // This object only handles named commands, not browser/page actions.
  if (ShouldIgnoreCommand(command_name))
    return;

  extensions::CommandService* command_service =
      extensions::CommandService::Get(profile_);
  // Add all the active keybindings (except page actions and browser actions,
  // which are handled elsewhere).
  extensions::CommandMap commands;
  if (!command_service->GetNamedCommands(
          extension->id(),
          extensions::CommandService::ACTIVE_ONLY,
          extensions::CommandService::REGULAR,
          &commands))
    return;
  extensions::CommandMap::const_iterator iter = commands.begin();
  for (; iter != commands.end(); ++iter) {
    if (!command_name.empty() && (iter->second.command_name() != command_name))
      continue;

    event_targets_[iter->second.accelerator()].push_back(
        std::make_pair(extension->id(), iter->second.command_name()));
    // Shortcuts except media keys have only one target in the list. See comment
    // about |event_targets_|.
    if (!extensions::CommandService::IsMediaKey(iter->second.accelerator()))
      DCHECK_EQ(1u, event_targets_[iter->second.accelerator()].size());
    if (event_targets_[iter->second.accelerator()].size() > 1)
      continue;   // It is already registered with the focus manager.

    focus_manager_->RegisterAccelerator(
        iter->second.accelerator(),
        ui::AcceleratorManager::kHighPriority, this);
  }
}

void ExtensionKeybindingRegistryViews::RemoveExtensionKeybindingImpl(
    const ui::Accelerator& accelerator,
    const std::string& command_name) {
  focus_manager_->UnregisterAccelerator(accelerator, this);
}

bool ExtensionKeybindingRegistryViews::AcceleratorPressed(
    const ui::Accelerator& accelerator) {
  return ExtensionKeybindingRegistry::NotifyEventTargets(accelerator);
}

bool ExtensionKeybindingRegistryViews::CanHandleAccelerators() const {
  return true;
}
