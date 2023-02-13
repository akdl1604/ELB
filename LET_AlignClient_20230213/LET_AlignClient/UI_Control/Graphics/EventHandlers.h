#pragma once
#include <functional>
using MouseEventCallback = std::function<void(CWnd* sender, CPoint pt)>;