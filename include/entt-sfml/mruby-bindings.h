#pragma once

#ifdef FOWL_ENTT_MRUBY

template<>
bool MRuby::to_mrb< sf::Color >(mrb_state* state, const sf::Color& input, mrb_value& output)
{
  mrb_value elements[4] = {
    mrb_fixnum_value(input.r),
    mrb_fixnum_value(input.g),
    mrb_fixnum_value(input.b),
    mrb_fixnum_value(input.a)
  };
  output = mrb_ary_new_from_values(state, 4, elements);
  return true;
}

template<>
bool MRuby::from_mrb< sf::Color >(mrb_state* state, mrb_value input, sf::Color& output)
{
  if(mrb_array_p(input))
  {
    auto len = ARY_LEN(mrb_ary_ptr(input));
    if(len < 3 || len > 4)
      return false;
    output.r = mrb_int(state, mrb_ary_entry(input, 0)) & 0xFF;
    output.g = mrb_int(state, mrb_ary_entry(input, 1)) & 0xFF;
    output.b = mrb_int(state, mrb_ary_entry(input, 2)) & 0xFF;
    output.a = len == 4 ? (mrb_int(state, mrb_ary_entry(input, 3)) & 0xFF) : 0xFF;
    return true;
  }
  return false;
}

template<>
bool MRuby::to_mrb< sf::Vector2f >(mrb_state* state, const sf::Vector2f& input, mrb_value& output)
{
  mrb_value elements[4] = {
    mrb_float_value(state, input.x),
    mrb_float_value(state, input.y)
  };
  output = mrb_ary_new_from_values(state, 2, elements);
  return true;
}

template<>
bool MRuby::from_mrb< sf::Vector2f >(mrb_state* state, mrb_value input, sf::Vector2f& output)
{
  if(mrb_array_p(input))
  {
    auto len = ARY_LEN(mrb_ary_ptr(input));
    if(len != 2)
      return false;
    output.x = mrb_to_flo(state, mrb_ary_entry(input, 0));
    output.y = mrb_to_flo(state, mrb_ary_entry(input, 1));
    return true;
  }
  return false;
}

template<>
bool MRuby::to_mrb< sf::FloatRect >(mrb_state* state, const sf::FloatRect& input, mrb_value& output)
{
  MRuby::HashBuilder b(state);
  b
    ("left", input.left)
    ("top", input.top)
    ("width", input.width)
    ("height", input.height);
  output = b.self;
  return true;
}

template<>
bool MRuby::from_mrb< sf::FloatRect >(mrb_state* state, mrb_value input, sf::FloatRect& output)
{
  if(mrb_hash_p(input))
  {
    MRuby::HashReader r(state, input);
    r
      ("left", output.left)
      ("top", output.top)
      ("width", output.width)
      ("height", output.height);
    return true;
  }
  return false;
}


#endif

