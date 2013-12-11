AM_CPPFLAGS = $(WX_CXXFLAGS) \
			-I$(top_srcdir)/src/include \
			-I$(top_srcdir)/src/sdk/wxscintilla/include

noinst_LTLIBRARIES = libsqbindings.la

libsqbindings_la_SOURCES = scriptbindings.cpp \
							sc_cb_vm.cpp \
                            sc_consts.cpp \
                            sc_dialog.cpp \
                            sc_globals.cpp \
                            sc_io.cpp \
                            sc_plugin.cpp \
                            sc_progress.cpp \
                            sc_util_dialogs.cpp \
                            sc_wxtypes.cpp \
							sq_wx/sq_util_dialogs.cpp \
							sq_wx/sq_wx_base_controls.cpp \
							sq_wx/sq_wx_constants.cpp \
							sq_wx/sq_wx.cpp \
							sq_wx/sq_wx_dialog.cpp \
							sq_wx/sq_wx_string.cpp
