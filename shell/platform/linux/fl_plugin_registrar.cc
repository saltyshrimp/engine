// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_plugin_registrar.h"
#include "flutter/shell/platform/linux/fl_plugin_registrar_private.h"
#include "flutter/shell/platform/linux/fl_view_private.h"

#include <gmodule.h>

struct _FlPluginRegistrar {
  GObject parent_instance;

  // View that plugin is controlling.
  FlView* view;

  // Messenger to communicate on.
  FlBinaryMessenger* messenger;
};

// Added here to stop the compiler from optimizing this function away.
G_MODULE_EXPORT GType fl_plugin_registrar_get_type();

G_DEFINE_TYPE(FlPluginRegistrar, fl_plugin_registrar, G_TYPE_OBJECT)

static void fl_plugin_registrar_dispose(GObject* object) {
  FlPluginRegistrar* self = FL_PLUGIN_REGISTRAR(object);

  if (self->view != nullptr) {
    g_object_remove_weak_pointer(G_OBJECT(self->view),
                                 reinterpret_cast<gpointer*>(&(self->view)));
    self->view = nullptr;
  }
  g_clear_object(&self->messenger);

  G_OBJECT_CLASS(fl_plugin_registrar_parent_class)->dispose(object);
}

static void fl_plugin_registrar_class_init(FlPluginRegistrarClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_plugin_registrar_dispose;
}

static void fl_plugin_registrar_init(FlPluginRegistrar* self) {}

FlPluginRegistrar* fl_plugin_registrar_new(FlView* view,
                                           FlBinaryMessenger* messenger) {
  g_return_val_if_fail(view == nullptr || FL_IS_VIEW(view), nullptr);
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);

  FlPluginRegistrar* self = FL_PLUGIN_REGISTRAR(
      g_object_new(fl_plugin_registrar_get_type(), nullptr));

  self->view = view;
  if (view != nullptr) {
    g_object_add_weak_pointer(G_OBJECT(view),
                              reinterpret_cast<gpointer*>(&(self->view)));
  }
  self->messenger = FL_BINARY_MESSENGER(g_object_ref(messenger));

  return self;
}

G_MODULE_EXPORT FlBinaryMessenger* fl_plugin_registrar_get_messenger(
    FlPluginRegistrar* self) {
  g_return_val_if_fail(FL_IS_PLUGIN_REGISTRAR(self), nullptr);

  return self->messenger;
}

G_MODULE_EXPORT FlView* fl_plugin_registrar_get_view(FlPluginRegistrar* self) {
  g_return_val_if_fail(FL_IS_PLUGIN_REGISTRAR(self), nullptr);

  return self->view;
}

G_MODULE_EXPORT void fl_plugin_registrar_register_view_factory(
    FlPluginRegistrar* self,
    FlPlatformViewFactory* factory,
    const gchar* view_type) {
  g_return_if_fail(FL_IS_PLUGIN_REGISTRAR(self));

  if (self->view) {
    FlPlatformViewsPlugin* plugin =
        fl_view_get_platform_views_plugin(self->view);
    fl_platform_views_plugin_register_view_factory(plugin, factory, view_type);
  }
}
