//
// Created by Jacob on 2/7/25.
//

#define TT_ALLOW_UNRELEASED
#include "../TermTk.hpp"
#include <unordered_map>

enum class Direction {
  None,
  Up,
  Down,
  Left,
  Right,
};

struct SnakeBit {
  Direction direction{Direction::None};
  int x{-1};
  int y{-1};
  std::string character{"█"};
};

struct Food {
  int x{-1};
  int y{-1};
};

struct Context {
  bool running{true};
  int score{0};
  Food food{};
  std::vector<SnakeBit> snake{};
  std::unordered_map<int, std::unordered_map<int, TermTk::String>> pixmap{};

  void draw() {
    TermTk::clear_screen();

    for (const auto& [y, row] : pixmap) {
      for (const auto& [x, cell] : row) {
        TermTk::set_cursor(y, x);
        std::cout << cell;
      }
    }

    TermTk::set_cursor(2, 2);
    TermTk::set_cursor(false);

    TermTk::finalize();
  }
};

TermTk::Size get_size() {
    auto size = TermTk::get_size();
    size.rows -= 1;
    size.cols -= 1;

    size.rows = std::min(50, size.rows);
    size.cols = std::min(50, size.cols);
    return size;
}

void draw_border(Context& ctx) {
  TermTk::clear_screen();

  auto size = get_size();

  if (size.rows < 10 || size.cols < 10) {
    std::cerr << "Terminal too small" << std::endl;
    throw std::runtime_error{"Terminal too small"};
  }

  ctx.pixmap[0][0] = "┌";
  ctx.pixmap[0][size.cols - 1] = "┐";
  ctx.pixmap[size.rows - 1][0] = "└";
  ctx.pixmap[size.rows - 1][size.cols - 1] = "┘";
  for (int i = 1; i < size.cols - 1; ++i) {
    ctx.pixmap[0][i] = "─";
    ctx.pixmap[size.rows - 1][i] = "─";
  }
  for (int i = 1; i < size.rows - 1; ++i) {
    ctx.pixmap[i][0] = "│";
    ctx.pixmap[i][size.cols - 1] = "│";
  }

  std::string score = std::to_string(ctx.score);
  for (int i = 0; i < score.size(); ++i) {
    ctx.pixmap[0][i + 1] = score[i];
  }
}

void clear_inner(Context& ctx) {
  auto size = get_size();
  for (int i = 1; i < size.rows - 1; ++i) {
    for (int j = 1; j < size.cols - 1; ++j) {
      ctx.pixmap[i][j] = " ";
    }
  }
}

void generate_food(Context& ctx) {
  auto size = get_size();
  int x = std::rand() % (size.cols - 2) + 1;
  int y = std::rand() % (size.rows - 2) + 1;
  ctx.food = {x, y};
}

void place_food(Context& ctx) {
  ctx.pixmap[ctx.food.y][ctx.food.x] = "█";
}

void clear_food(Context& ctx) {
  ctx.pixmap[ctx.food.y][ctx.food.x] = " ";
}

void append_snake(Context& ctx) {
    clear_inner(ctx);
    auto size = TermTk::get_size();

    for (const auto& it : ctx.snake) {
        ctx.pixmap[it.y][it.x] = it.character;
    }
}

void draw_ded(Context& ctx) {
    clear_inner(ctx);

    auto size = get_size();
    std::string text = "Game Over";
    int start = (size.cols / 2) - (text.size() / 2);
    ctx.pixmap[size.rows / 2][start] = text;
    text = "Score: " + std::to_string(ctx.score);
    start = (size.cols / 2) - (text.size() / 2);
    ctx.pixmap[size.rows / 2 + 1][start] = text;
    ctx.running = false;
}

int main() {
    Context ctx{};

    auto size = get_size();
    ctx.snake.push_back({Direction::None, size.cols / 2, size.rows / 2});

    bool has_drawn_once{false};
    while (ctx.running) {
        draw_border(ctx);
        append_snake(ctx);

        if (ctx.food.x == -1) {
          generate_food(ctx);
        }
        place_food(ctx);

        TermTk::Key key = TermTk::get_key();
        if (key == TermTk::Key::Backspace) {
          throw std::runtime_error{"User quit"};
        }

        if (key == TermTk::Key::Up && (ctx.snake[0].direction != Direction::Down || ctx.snake.size() == 1)) {
          ctx.snake[0].direction = Direction::Up;
        } else if (key == TermTk::Key::Down && (ctx.snake[0].direction != Direction::Up || ctx.snake.size() == 1)) {
          ctx.snake[0].direction = Direction::Down;
        } else if (key == TermTk::Key::Left && (ctx.snake[0].direction != Direction::Right || ctx.snake.size() == 1)) {
          ctx.snake[0].direction = Direction::Left;
        } else if (key == TermTk::Key::Right && (ctx.snake[0].direction != Direction::Left || ctx.snake.size() == 1)) {
          ctx.snake[0].direction = Direction::Right;
        }

        if (ctx.snake[0].direction == Direction::Up) {
            ctx.snake.insert(ctx.snake.begin(), {ctx.snake[0].direction, ctx.snake[0].x, ctx.snake[0].y - 1});
        } else if (ctx.snake[0].direction == Direction::Down) {
            ctx.snake.insert(ctx.snake.begin(), {ctx.snake[0].direction, ctx.snake[0].x, ctx.snake[0].y + 1});
        } else if (ctx.snake[0].direction == Direction::Left) {
            ctx.snake.insert(ctx.snake.begin(), {ctx.snake[0].direction, ctx.snake[0].x - 1, ctx.snake[0].y});
        } else if (ctx.snake[0].direction == Direction::Right) {
            ctx.snake.insert(ctx.snake.begin(), {ctx.snake[0].direction, ctx.snake[0].x + 1, ctx.snake[0].y});
        }

        if (ctx.snake.size() > ctx.score + 1) {
            ctx.snake.pop_back();
        }

        if (ctx.food.x == ctx.snake[0].x && ctx.food.y == ctx.snake[0].y) {
           ++ctx.score;
           ctx.food = {-1, -1};
        }

        if (ctx.snake.size() > 2) {
            for (int i = 1; i < ctx.snake.size(); ++i) {
                if (ctx.snake[i].x == ctx.snake[0].x && ctx.snake[i].y == ctx.snake[0].y) {
                  draw_ded(ctx);
                }
            }
        }

        if (ctx.snake[0].x >= size.cols - 1) {
            ctx.snake[0].x = 1;
        } else if (ctx.snake[0].x <= 0) {
            ctx.snake[0].x = size.cols - 2;
        } else if (ctx.snake[0].y >= size.rows - 1) {
            ctx.snake[0].y = 1;
        } else if (ctx.snake[0].y <= 0) {
            ctx.snake[0].y = size.rows - 2;
        }

        ctx.draw();
        TermTk::sleep(100);
    }
    std::cin.get();
}