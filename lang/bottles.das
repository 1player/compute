bottle := func(i) {
  if i == 0 {
    "no more bottles of beer"
  } else if i == 1 {
    "1 bottle of beer"
  } else {
    i.inspect.concat(" bottles of beer")
  }
}

i := 99
loop i > 0 {
  bottle(i).concat(" on the wall, ").concat(bottle(i)).concat(",").println
  "take one down, pass it around,".println
  bottle(i - 1).concat(" on the wall.").println
  "".println
  
  i = i - 1
}
