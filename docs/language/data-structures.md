 = 0
while i < len(enemies) {
    enemies[i].update()
    i = i + 1
}
```

### Reverse Iteration (for removal)

When removing items during iteration, go backwards:

```pixel
i = len(bullets) - 1
while i >= 0 {
    if bullets[i].off_screen {
        remove