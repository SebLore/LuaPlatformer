local e = ecs.create()

ecs.add(e, "Position", { x = 100, y = 150 })
ecs.add(e, "Circle",    { r = 25 })

function Update(dt)
  print("Updating")
  local p = ecs.get(e, "Position")
  p.x = p.x + 60 * dt
  ecs.set(e, "Position", p)
end

function Draw()
  print("Drawing")
  local p = ecs.get(e, "Position")
  local c = ecs.get(e, "Circle")
  ClearBackgroundRGBA(20,20,30,255)
  DrawCircle(p.x, p.y, c.r)
end
