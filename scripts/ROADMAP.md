# Local Hermes / MLX roadmap

## Done — task router gateway

Hermes talks to one OpenAI-compatible URL (`http://127.0.0.1:4000/v1`). The gateway turns thinking off, strips gpt-oss Harmony analysis tokens, drops `response_format` on the main path, then routes:

- main chat / tools → gpt-oss-20b (`:8080`, mlx_lm)
- titles / compression → Qwen3-8B 4bit (`:8081`, mlx_lm) via model alias `qwen-fast`

Start the stack with `scripts/serve-all.sh`. `scripts/hermes.sh` starts it if `:4000` is down. Roll back to Qwen 27B with `scripts/serve-qwen.sh` on `:8080`.

## Next

- Optional classifier so some user turns skip 27B
- Raise `compression.threshold` toward 0.65 if 27B KV pressure shows up
