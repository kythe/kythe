require 'nokogiri'

module Jekyll
  class TOCGenerator < Jekyll::Generator
    def generate(site)
      site.pages.each do |page|
        maybeTOC page
      end
      site.collections.each do |name, coll|
        coll.docs.each do |page|
          maybeTOC page
        end
      end
    end

    private

    def maybeTOC(page)
      if page.data['toc']
        maxDepth = page.data['toclevels'] ? page.data['toclevels'].to_i : 2
        toc = parseTOC(Nokogiri::HTML(page.content), maxDepth)
        page.data['toc'] = renderTOC(toc)
      end
    end

    def renderTOC(toc)
      html = '<div class="toc">'
      html += tocList(toc)
      return html + '</div>'
    end

    def tocList(toc)
      html = '<ul>'
      toc.each do |h|
        header = h['header']
        children = h['children']

        html += '<li><a href="#' + header['id'] + '">' + header.children.text  + '</a>'
        if not children.empty? then
          html += tocList children
        end
        html += '</li>'
      end
      return html + '</ul>'
    end

    def parseTOC(doc, maxDepth)
      1.upto(6).each do |level|
        hdrs = doc.css("h#{level}")
        unless hdrs.empty?
          return hdrs.map {|h| toc(h, level, level+maxDepth-1) }
        end
      end
      []
    end

    def toc(h, level, maxLevel)
      return {'header'=>h, 'children'=>[]} if level == maxLevel
      # Number of following headers at the same level
      ct = h.xpath("count(following::h#{level})")
      # All headers of the next level before the next header of the same level
      children = h.xpath("following::h#{level+1}[count(following::h#{level})=#{ct}]")
      {'header'=>h, 'children'=>children.map {|c| toc(c, level+1, maxLevel) }}
    end
  end
end
