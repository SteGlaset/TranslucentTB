#include <cassert>
#include "../Common/neovidetheme.hpp"
#include "../ExplorerTAP/taskbartransition.hpp"

int main()
{
	using namespace NeovideTheme;
	static_assert(sizeof(Packet) == 16);
	assert(Valid({ 1, 123, 0, 1 })); // black is a valid theme
	assert(!Valid({ 2, 123, 0, 1 }));
	assert(!Valid({ 1, 0, 0, 1 }));
	assert(!Valid({ 1, 123, 0x1000000, 1 }));
	assert(!Valid({ 1, 123, 0, 2 }));
	FocusContext context;
	assert(context.Observe(10, false) == 10);
	assert(context.Observe(20, true) == 10);
	assert(context.Observe(30, false) == 30);
	assert(context.Observe(20, true) == 30);
	assert(Select(true, true, false, false).Tint);
	assert(!Select(true, true, false, false).HideBorder);
	assert(Select(true, true, false, true).HideBorder);
	assert(!Select(true, true, true, true).Tint);
	assert(!Select(false, true, false, true).Tint);
	assert(!Select(true, false, false, true).Tint);
	TaskbarTransition transition; // compile the exact C++/WinRT implementation
	transition.Hide();
	transition.Reset();
}
