##
## MIWMrb Test
##

assert("MIWMrb#hello") do
  t = MIWMrb.new "hello"
  assert_equal("hello", t.hello)
end

assert("MIWMrb#bye") do
  t = MIWMrb.new "hello"
  assert_equal("hello bye", t.bye)
end

assert("MIWMrb.hi") do
  assert_equal("hi!!", MIWMrb.hi)
end
