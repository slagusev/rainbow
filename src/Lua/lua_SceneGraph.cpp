#include "Lua/LuaHelper.h"
#include "Lua/lua_Animation.h"
#include "Lua/lua_Label.h"
#include "Lua/lua_SceneGraph.h"
#include "Lua/lua_Sprite.h"
#include "Lua/lua_SpriteBatch.h"

namespace
{
	enum CastingMethod
	{
		kCastingUnsafe,
		kCastingSafe
	};

	template<class T, CastingMethod C>
	struct ToUserData;

	template<class T>
	struct ToUserData<T, kCastingSafe>
	{
		static T* Cast(lua_State *L, const int n)
		{
			return static_cast<T*>(luaL_checkudata(L, n, T::class_name));
		}
	};

	template<class T>
	struct ToUserData<T, kCastingUnsafe>
	{
		static T* Cast(lua_State *L, const int n)
		{
			return static_cast<T*>(lua_touserdata(L, n));
		}
	};

	SceneGraph::Node* check_node(lua_State *L, const int n)
	{
		SceneGraph::Node *node = static_cast<SceneGraph::Node*>(lua_touserdata(L, n));
		LUA_CHECK(L, node, "rainbow.scenegraph: Invalid node specified");
		return node;
	}

	template<class T, CastingMethod C>
	int add_child(SceneGraph::Node *root, lua_State *L)
	{
		R_ASSERT(lua_gettop(L) == 1 || lua_gettop(L) == 2, "Invalid parameters");

		// Ensure it's not a nil value.
		const int n = lua_gettop(L);
		LUA_CHECK(L, lua_type(L, n) != LUA_TNIL, "rainbow.scenegraph: Invalid child node specified");

		// Retrieve Lua wrapper.
		lua_rawgeti(L, n, 0);
		T *obj = ToUserData<T, C>::Cast(L, -1);
		lua_pop(L, 1);

		// Retrieve and add element.
		SceneGraph::Node *node = (n == 1) ? root : check_node(L, 1);
		R_ASSERT(node, "This shouldn't ever happen.");
		lua_pushlightuserdata(L, node->add_child(obj->get()));

		return 1;
	}
}

namespace Rainbow
{
	namespace Lua
	{
		typedef Bind<SceneGraph, ::SceneGraph::Node, kBindTypeWeak> LuaSceneGraph;

		template<>
		const char LuaSceneGraph::class_name[] = "scenegraph";

		template<>
		const Method<SceneGraph> LuaSceneGraph::methods[] = {
			{ "add_animation",  &SceneGraph::add_animation },
			{ "add_batch",      &SceneGraph::add_batch },
			{ "add_drawable",   &SceneGraph::add_drawable },
			{ "add_label",      &SceneGraph::add_label },
			{ "add_node",       &SceneGraph::add_node },
			{ "enable",         &SceneGraph::enable },
			{ "disable",        &SceneGraph::disable },
			{ "remove",         &SceneGraph::remove },
			{ "set_parent",     &SceneGraph::set_parent },
			{ "move",           &SceneGraph::move },
			{ 0, 0 }
		};

		SceneGraph* SceneGraph::create(lua_State *L, ::SceneGraph::Node *root)
		{
			lua_pushlstring(L, class_name, sizeof(class_name) / sizeof(char) - 1);
			lua_createtable(L, 1, 0);  // scenegraph = {}

			SceneGraph *ptr = static_cast<SceneGraph*>(lua_newuserdata(L, sizeof(SceneGraph)));
			luaL_newmetatable(L, class_name);  // mt = {}
			lua_setmetatable(L, -2);           // setmetatable(udata, mt)
			lua_rawseti(L, -2, 0);             // scenegraph[0] = udata

			lua_createtable(L, 0, 2);  // mt = {}
			lua_pushliteral(L, "__index");
			lua_createtable(L, 0, 16);
			for (int i = 0; methods[i].name; ++i)
			{
				lua_pushstring(L, methods[i].name);
				lua_pushnumber(L, i);
				lua_pushcclosure(L, &thunk<SceneGraph>, 1);
				lua_rawset(L, -3);
			}
			lua_rawset(L, -3);        // mt.__index = { .. }
			lua_pushliteral(L, "__metatable");
			lua_createtable(L, 0, 0);
			lua_rawset(L, -3);        // mt.__metatable = {}
			lua_setmetatable(L, -2);  // setmetatable(scenegraph, mt)

			lua_rawset(L, -3);        // rainbow.scenegraph = scenegraph

			return new (ptr) SceneGraph(root);
		}

		void SceneGraph::destroy(lua_State *L, SceneGraph *scenegraph)
		{
			lua_getglobal(L, "rainbow");
			lua_pushlstring(L, class_name, sizeof(class_name) / sizeof(char) - 1);
			lua_pushnil(L);
			lua_rawset(L, -3);
			lua_pop(L, 1);
			scenegraph->~SceneGraph();
		}

		SceneGraph::SceneGraph(::SceneGraph::Node *root) : Bind(root) { }

		int SceneGraph::add_animation(lua_State *L)
		{
			return add_child<Animation, kCastingSafe>(this->ptr, L);
		}

		int SceneGraph::add_batch(lua_State *L)
		{
			return add_child<SpriteBatch, kCastingSafe>(this->ptr, L);
		}

		int SceneGraph::add_drawable(lua_State *L)
		{
			return add_child<Drawable, kCastingUnsafe>(this->ptr, L);
		}

		int SceneGraph::add_label(lua_State *L)
		{
			return add_child<Label, kCastingSafe>(this->ptr, L);
		}

		int SceneGraph::add_node(lua_State *L)
		{
			LUA_ASSERT(lua_gettop(L) == 0 || lua_gettop(L) == 1,
			           "rainbow.scenegraph:add_node([parent])");

			::SceneGraph::Node *node = (!lua_gettop(L)) ? this->ptr : check_node(L, 1);
			R_ASSERT(node, "This shouldn't ever happen.");
			lua_pushlightuserdata(L, node->add_child(new ::SceneGraph::Node()));
			return 1;
		}

		int SceneGraph::enable(lua_State *L)
		{
			LUA_ASSERT(lua_gettop(L) == 1, "rainbow.scenegraph:enable(node)");

			check_node(L, 1)->enabled = true;
			return 0;
		}

		int SceneGraph::disable(lua_State *L)
		{
			LUA_ASSERT(lua_gettop(L) == 1, "rainbow.scenegraph:disable(node)");

			check_node(L, 1)->enabled = false;
			return 0;
		}

		int SceneGraph::remove(lua_State *L)
		{
			LUA_ASSERT(lua_gettop(L) == 1, "rainbow.scenegraph:remove(node)");

			check_node(L, 1)->remove();
			return 0;
		}

		int SceneGraph::set_parent(lua_State *L)
		{
			LUA_ASSERT(lua_gettop(L) == 2, "rainbow.scenegraph:set_parent(parent, child)");

			::SceneGraph::Node *child = check_node(L, 2);
			child->set_parent(check_node(L, 1));
			return 0;
		}

		int SceneGraph::move(lua_State *L)
		{
			LUA_ASSERT(lua_gettop(L) == 3, "rainbow.scenegraph:move(node, x, y)");

			::SceneGraph::Node *node = check_node(L, 1);
			const Vec2f delta(luaR_tonumber(L, 2), luaR_tonumber(L, 3));
			node->move(delta);
			return 0;
		}
	}
}
