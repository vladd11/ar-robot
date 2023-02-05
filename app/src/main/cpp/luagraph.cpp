#include "luagraph.h"
#include <vector>
#include <stack>
#include <queue>
#include <android/log.h>

#define TAG "LuaGraph"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

extern "C" {
#include "lauxlib.h"
#include "lualib.h"
}

class Node {
public:
  int mVertex;
  float mWeight;

  Node(int v, float w) {
    mVertex = v;
    mWeight = w;
  }

  bool operator<(const Node &rhs) const {
    return mWeight < rhs.mWeight;
  }

  bool operator>(const Node &rhs) const {
    return rhs < *this;
  }

  bool operator<=(const Node &rhs) const {
    return !(rhs < *this);
  }

  bool operator>=(const Node &rhs) const {
    return !(*this < rhs);
  }
};

void recoverPath(std::stack<int> *stack, std::vector<int> prev, int src,
                 int to) { // NOLINT(misc-no-recursion) DFS
  if (to == src) return;
  if (to >= prev.size() || prev[to] == INT32_MAX) {
    LOGD("No route to point");
    return;
  }
  stack->push(to);
  recoverPath(stack, prev, src, prev[to]);
}

void
dijkstra(int V, std::vector<std::vector<Node *>> graph, int src, int dst, std::stack<int> *route) {
  std::vector<float> dist(V, FLT_MAX);
  std::vector<int> path(V, INT32_MAX);

  dist[src] = 0;

  std::priority_queue<Node, std::vector<Node>, std::greater<>> nodes;
  nodes.push(Node(src, 0));

  while (!nodes.empty()) {
    int u = nodes.top().mVertex;
    nodes.pop();

    for (Node *node: graph[u]) {

      // If there is shorted path to v through u.
      if (dist[node->mVertex] > dist[u] + node->mWeight) {
        // Updating distance of v
        dist[node->mVertex] = dist[u] + node->mWeight;
        nodes.push(Node(node->mVertex, dist[node->mVertex]));
        path[node->mVertex] = u;
      }
    }
  }

  recoverPath(route, path, src, dst);
}

struct Graph {
  std::vector<std::vector<Node *>> *vector;
};

int createGraph(lua_State *L) {
  int V = (int) luaL_checkinteger(L, 1);

  auto *userdata = (Graph *) lua_newuserdata(L, sizeof(Graph));
  userdata->vector = new std::vector<std::vector<Node *>>(V, std::vector<Node *>());

  return 1;
}

int deleteGraph(lua_State *L) {
  auto *ptr = static_cast<Graph *>(lua_touserdata(L, 1));

  for (const std::vector<Node *> &nodes: *ptr->vector) {
    for (Node *node: nodes) {
      delete node;
    }
  }
  ptr->vector->clear();
  ptr->vector->shrink_to_fit();

  delete ptr->vector;
  return 0;
}

int buildRoute(lua_State *L) {
  auto *ptr = static_cast<Graph *>(lua_touserdata(L, 1));
  int src = static_cast<int>(luaL_checkinteger(L, 2));
  int dst = static_cast<int>(luaL_checkinteger(L, 3));

  std::stack<int> stack;
  dijkstra((int) ptr->vector->size(), *ptr->vector, src, dst, &stack);

  lua_createtable(L, (int) stack.size(), 0);

  int i = 0;
  while (!stack.empty()) {
    lua_pushinteger(L, stack.top());
    stack.pop();
    lua_rawseti(L, -2, ++i); /* In lua indices start at 1 */
  }
  return 1;
}

int connect(lua_State *L) {
  auto ptr = static_cast<Graph *>(lua_touserdata(L, 1));
  long long from = luaL_checkinteger(L, 2);
  long long to = luaL_checkinteger(L, 3);
  auto w = (float) luaL_checknumber(L, 4);

  ptr->vector->operator[](from).emplace_back(new Node(to, w));
  ptr->vector->operator[](to).emplace_back(new Node(from, w));

  return 0;
}

int setAdjWeight(lua_State *L) {
  auto ptr = static_cast<Graph *>(lua_touserdata(L, 1));
  long long from = luaL_checkinteger(L, 2);
  long long to = luaL_checkinteger(L, 3);
  auto w = (float) luaL_checknumber(L, 4);

  ptr->vector->operator[](from).operator[](to)->mWeight = w;

  return 0;
}

int getAdjCount(lua_State *L) {
  auto ptr = static_cast<Graph *>(lua_touserdata(L, 1));
  int idx = (int) luaL_checkinteger(L, 2);
  lua_pushinteger(L, (int) ptr->vector->operator[](idx).size());
  return 1;
}

int getAdj(lua_State *L) {
  auto ptr = static_cast<Graph *>(lua_touserdata(L, 1));
  int x = (int) luaL_checkinteger(L, 2);
  int y = (int) luaL_checkinteger(L, 3);

  lua_pushinteger(L, ptr->vector->operator[](x).operator[](y)->mVertex);
  return 1;
}