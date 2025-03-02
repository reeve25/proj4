#include <gtest/gtest.h>
#include "../include/StringUtils.h"
#include <string>

TEST(StringUtilsTest, SliceTest){
    std::string str = "My name is Logan";
    std::string out1 = StringUtils::Slice(str, 0, 2);
    EXPECT_EQ(out1, "My");
    std::string out2 = StringUtils::Slice(str, 0, str.length());
    EXPECT_EQ(out2, str);
    std::string out3 = StringUtils::Slice(str, 3, 7);
    EXPECT_EQ(out3, "name");
    std::string out4 = StringUtils::Slice(str, 11, str.length());
    EXPECT_EQ(out4, "Logan");
}

TEST(StringUtilsTest, Capitalize){
    std::string str1 = "  hello";
    std::string str2 = "How are23";
    std::string str3 = "yOU DOING";
    EXPECT_EQ(StringUtils::Capitalize(str1), "  Hello");
    EXPECT_EQ(StringUtils::Capitalize(str2), str2);
    EXPECT_EQ(StringUtils::Capitalize(str3), "YOU DOING");
}

TEST(StringUtilsTest, Upper){
    std::string str1 = "hElLo";
    std::string str2 = "how are23";
    std::string str3 = "YOU DOING";
    EXPECT_EQ(StringUtils::Upper(str1), "HELLO");
    EXPECT_EQ(StringUtils::Upper(str2), "HOW ARE23");
    EXPECT_EQ(StringUtils::Upper(str3), str3);
}

TEST(StringUtilsTest, Lower){
    std::string str1 = "hElLO";
    std::string str2 = "How 64 are";
    std::string str3 = "YOU DOING";
    std::string str4 = "today 343djfdkjfkdsjf43768374";
    EXPECT_EQ(StringUtils::Lower(str1), "hello");
    EXPECT_EQ(StringUtils::Lower(str2), "how 64 are");
    EXPECT_EQ(StringUtils::Lower(str3), "you doing");
    EXPECT_EQ(StringUtils::Lower(str4), str4);
}

TEST(StringUtilsTest, LStrip){
    std::string str1 = "      Hello";
    std::string str2 = "    5how are 23   ";
    std::string str3 = "YOU DOING    ";
    std::string str4 = "";
    EXPECT_EQ(StringUtils::LStrip(str1), "Hello");
    EXPECT_EQ(StringUtils::LStrip(str2), "5how are 23   ");
    EXPECT_EQ(StringUtils::LStrip(str3), str3);
    EXPECT_EQ(StringUtils::LStrip(str4), str4);
}

TEST(StringUtilsTest, RStrip){
    std::string str1 = "      Hello";
    std::string str2 = "    5how are 23   ";
    std::string str3 = "YOU DOING    ";
    std::string str4 = "";
    EXPECT_EQ(StringUtils::RStrip(str1), str1);
    EXPECT_EQ(StringUtils::RStrip(str2), "    5how are 23");
    EXPECT_EQ(StringUtils::RStrip(str3), "YOU DOING");
    EXPECT_EQ(StringUtils::RStrip(str4), str4);
}

TEST(StringUtilsTest, Strip){
    std::string str1 = "      Hello";
    std::string str2 = "    5how are 23   ";
    std::string str3 = "YOU DOING    ";
    std::string str4 = "";
    EXPECT_EQ(StringUtils::Strip(str1), "Hello");
    EXPECT_EQ(StringUtils::Strip(str2), "5how are 23");
    EXPECT_EQ(StringUtils::Strip(str3), "YOU DOING");
    EXPECT_EQ(StringUtils::Strip(str4), str4);
}

TEST(StringUtilsTest, Center){
    EXPECT_EQ(StringUtils::Center("hello", 10), "  hello   ");
    EXPECT_EQ(StringUtils::Center("there", 9), "  there  ");
    EXPECT_EQ(StringUtils::Center("test", 8, '-'), "--test--");
    EXPECT_EQ(StringUtils::Center("test", 9, '-'), "--test---");
    EXPECT_EQ(StringUtils::Center("  ", 5, ' '), "     ");
}

TEST(StringUtilsTest, RJust){
    EXPECT_EQ(StringUtils::RJust("hello", 8), "   hello");
    EXPECT_EQ(StringUtils::RJust("there", 9), "    there");
    EXPECT_EQ(StringUtils::RJust("test", 6, '-'), "--test");
    EXPECT_EQ(StringUtils::RJust("test", 9, 't'), "ttttttest");
    EXPECT_EQ(StringUtils::RJust("  ", 5, ' '), "     ");
}

TEST(StringUtilsTest, LJust){
    EXPECT_EQ(StringUtils::LJust("hello", 8), "hello   ");
    EXPECT_EQ(StringUtils::LJust("there", 9), "there    ");
    EXPECT_EQ(StringUtils::LJust("test", 6, '-'), "test--");
    EXPECT_EQ(StringUtils::LJust("test", 9, 't'), "testttttt");
    EXPECT_EQ(StringUtils::LJust("  ", 5, ' '), "     ");
}

TEST(StringUtilsTest, Replace){
    EXPECT_EQ(StringUtils::Replace("hello world", "world", "friend"), "hello friend");
    EXPECT_EQ(StringUtils::Replace("aaaa", "aa", "aaa"), "aaaaaa");
    EXPECT_EQ(StringUtils::Replace("test", "", "x"), "test");
    EXPECT_EQ(StringUtils::Replace("test", "t", ""), "es");
}

TEST(StringUtilsTest, Split){
    std::string str1 = "hello world how ";
    std::vector<std::string> compVect = {"hello", "world", "how", ""};
    EXPECT_EQ(StringUtils::Split(str1, " "), compVect);
    std::string str2 = "hello";
    compVect = {"h", "e", "l", "l", "o"};
    EXPECT_EQ(StringUtils::Split(str2), compVect);
    std::string str3 = "";
    compVect = {""};
    EXPECT_EQ(StringUtils::Split(str3), compVect);
    std::string str = "A  test";  // two spaces between A and test
    std::vector<std::string> expected = {"A", " ", "t", "e", "s", "t"};
    EXPECT_EQ(StringUtils::Split(str, "\n"), expected);
}

TEST(StringUtilsTest, Join){
    std::vector<std::string> strings = {"hello", "world", "", "how"};
    EXPECT_EQ(StringUtils::Join(" ", strings), "hello world  how");
    EXPECT_EQ(StringUtils::Join(",", strings), "hello,world,,how");
    EXPECT_EQ(StringUtils::Join("", strings), "helloworldhow");

}

TEST(StringUtilsTest, ExpandTabs){
    EXPECT_EQ(StringUtils::ExpandTabs("hello\tworld", 4), "hello    world");
    EXPECT_EQ(StringUtils::ExpandTabs("\thello\tworld\t", 2), "  hello  world  ");
    EXPECT_EQ(StringUtils::ExpandTabs("a\t\tb", 2), "a    b");
    EXPECT_EQ(StringUtils::ExpandTabs("", 4), "");
    EXPECT_EQ(StringUtils::ExpandTabs("no tabs", 4), "no tabs");
}

TEST(StringUtilsTest, EditDistance){
    EXPECT_EQ(StringUtils::EditDistance("kitten", "sitting"), 3);
    EXPECT_EQ(StringUtils::EditDistance("kitten", "kitten"), 0);
    EXPECT_EQ(StringUtils::EditDistance("", "test"), 4);
    EXPECT_EQ(StringUtils::EditDistance("test", ""), 4);
    EXPECT_EQ(StringUtils::EditDistance("HeLLo", "hello", false), 3);
    EXPECT_EQ(StringUtils::EditDistance("Hello", "hello", true), 0);
}
