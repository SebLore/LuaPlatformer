#pragma once

// Helper macro to avoid writing 'struct nameTag{};' for
#define TAG(name) struct name##Tag {};

TAG(Player)
TAG(Render)