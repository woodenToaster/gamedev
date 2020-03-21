internal void drawOpenGLRenderGroup(void *renderer, RenderGroup *group)
{
    for (int layerIndex = 0; layerIndex < RenderLayer_Count; ++layerIndex)
    {
        for (u32 baseAddress = 0; baseAddress < group->bufferSize;)
        {
            RenderEntryHeader *header = (RenderEntryHeader*)(group->bufferBase + baseAddress);
            baseAddress += sizeof(*header);
            
            void *data = (u8*)header + sizeof(*header);
            switch (header->type)
            {
                case RenderEntryType_RenderEntryRect:
                {
                    RenderEntryRect *entry = (RenderEntryRect*)data;
                    if (entry->layer == layerIndex)
                    {
                        rendererAPI.renderRect(renderer, entry->dest, entry->color);
                    }
                    baseAddress += sizeof(*entry);
                } break;
                case RenderEntryType_RenderEntryFilledRect:
                {
                    RenderEntryFilledRect *entry = (RenderEntryFilledRect*)data;
                    if (entry->layer == layerIndex)
                    {
                        rendererAPI.renderFilledRect(renderer, entry->dest, entry->color);
                    }
                    baseAddress += sizeof(*entry);
                } break;
                
                case RenderEntryType_RenderEntrySprite:
                {
                    RenderEntrySprite *entry = (RenderEntrySprite*)data;
                    if (entry->layer == layerIndex)
                    {
                        rendererAPI.renderSprite(renderer, entry->sheet, entry->source, entry->dest);
                    }
                    baseAddress += sizeof(*entry);
                } break;
                case RenderEntryType_RenderEntryLoadedBitmap:
                {
                    RenderEntryLoadedBitmap *entry = (RenderEntryLoadedBitmap *)data;
                    if (entry->layer == layerIndex)
                    {
                        rendererAPI.renderBitmap(renderer, entry->bitmap, entry->source, entry->dest);
                    }
                    baseAddress += sizeof(*entry);
                } break;
                default:
                {
                    InvalidCodePath;
                }
            }
        }
    }
}