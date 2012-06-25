// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_EXTENSION_UNINSTALL_DIALOG_H_
#define CHROME_BROWSER_EXTENSIONS_EXTENSION_UNINSTALL_DIALOG_H_
#pragma once

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/extensions/image_loading_tracker.h"
#include "chrome/browser/ui/browser_list.h"
#include "ui/gfx/image/image_skia.h"

class MessageLoop;

namespace extensions {
class Extension;
}

class ExtensionUninstallDialog
    : public ImageLoadingTracker::Observer,
      public BrowserList::Observer,
      public base::SupportsWeakPtr<ExtensionUninstallDialog> {
 public:
  class Delegate {
   public:
    // We call this method to signal that the uninstallation should continue.
    virtual void ExtensionUninstallAccepted() = 0;

    // We call this method to signal that the uninstallation should stop.
    virtual void ExtensionUninstallCanceled() = 0;

   protected:
    virtual ~Delegate() {}
  };

  // Creates a platform specific implementation of ExtensionUninstallDialog.
  // |browser| can be NULL only for Ash when this is used with the applist
  // window.
  static ExtensionUninstallDialog* Create(Browser* browser, Delegate* delegate);

  virtual ~ExtensionUninstallDialog();

  // This is called to verify whether the uninstallation should proceed.
  // Starts the process of showing a confirmation UI, which is split into two.
  // 1) Set off a 'load icon' task.
  // 2) Handle the load icon response and show the UI (OnImageLoaded).
  void ConfirmUninstall(const extensions::Extension* extension);

 protected:
  // Constructor used by the derived classes.
  ExtensionUninstallDialog(Browser* browser, Delegate* delegate);

  Browser* browser_;

  // The delegate we will call Accepted/Canceled on after confirmation dialog.
  Delegate* delegate_;

  // The extension we are showing the dialog for.
  const extensions::Extension* extension_;

  // The extensions icon.
  gfx::ImageSkia icon_;

 private:
  // Sets the icon that will be used in the dialog. If |icon| contains an empty
  // image, then we use a default icon instead.
  void SetIcon(const gfx::Image& image);

  // ImageLoadingTracker::Observer:
  virtual void OnImageLoaded(const gfx::Image& image,
                             const std::string& extension_id,
                             int index) OVERRIDE;

  // BrowserList::Observer
  virtual void OnBrowserRemoved(Browser* browser) OVERRIDE;

  // Displays the prompt. This should only be called after loading the icon.
  // The implementations of this method are platform-specific.
  virtual void Show() = 0;

  MessageLoop* ui_loop_;

  // Keeps track of extension images being loaded on the File thread for the
  // purpose of showing the dialog.
  scoped_ptr<ImageLoadingTracker> tracker_;

  DISALLOW_COPY_AND_ASSIGN(ExtensionUninstallDialog);
};

#endif  // CHROME_BROWSER_EXTENSIONS_EXTENSION_UNINSTALL_DIALOG_H_
