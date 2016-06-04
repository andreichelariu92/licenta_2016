print("Hello world")
require"testLib"
print(testLib.get())
testLib.add(5)
print(testLib.get())

messages = testLib.getMessages()
for _, message in pairs(messages) do
    print(message.id, message.buffer)
    print("\n")
end
